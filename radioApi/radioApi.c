/****************************************************************************
 ** Released under The MIT License (MIT). This code comes without warranty, **
 ** but if you use it you must provide attribution back to David's Blog     **
 ** at http://www.codehosting.net   See the LICENSE file for more details.  **
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h> // needed to run server on a new thread
#include <termios.h> // needed for unbuffered_getch()
#include <signal.h>  // needed for kill()
#ifndef __OPENWRT__
  #include <math.h>
#endif

#include "dwebsvr.h"

#define FILE_CHUNK_SIZE 1024
#define BIGGEST_FILE 104857600 // 100 Mb

#ifdef __arm__
  #define AUDIO "PCM"
  #define ADJUST "-a3"
#elif __OPENWRT__
  #define AUDIO "Headphone"
  #define ADJUST "-a-6"
#else
  #define AUDIO "Master"
  #define ADJUST "-a3"
#endif

struct {
    char *ext;
    char *filetype;
} extensions [] = {
  {"gif", "image/gif" },  
  {"jpg", "image/jpg" }, 
  {"jpeg","image/jpeg"},
  {"png", "image/png" },  
  {"ico", "image/ico" },  
  {"zip", "image/zip" },
  {"gz",  "image/gz"  },   
  {"tar", "image/tar" },  
  {"htm", "text/html" },  
  {"html","text/html" },  
  {"js","text/javascript" },
  {"txt","text/plain" },
  {"css","text/css" },
  {"map","application/json" },
  {"woff","application/font-woff" },
  {"woff2","application/font-woff2" },
  {"ttf","application/font-sfnt" },
  {"svg","image/svg+xml" },
  {"eot","application/vnd.ms-fontobject" },
  {"mp4","video/mp4" },
  {0,0}
};

int set_vol(int volume);
void send_response(struct hitArgs *args, char*, char*, http_verb);
void log_filter(log_type, char*, char*, int);
void null_log(log_type, char*, char*, int);
void kill_player();
void send_api_response(struct hitArgs *args, char*, char*);
void send_file_response(struct hitArgs *args, char*, char*, int);
void execpiped(char **cmdfrom, char **cmdto, int *frompid, int *topid);
int run(char **cmd);

struct termios original_settings;
pthread_t server_thread_id;
int vol = 5, lastwgetpid = 0, lastplayerpid = 0;
FILE *err = NULL;

void* server_thread(void *args)
{
  char *arg = (char*)args;
  dwebserver(atoi(arg), &send_response, &log_filter);
  return NULL;
}

void close_down()
{
  tcsetattr(STDIN_FILENO, TCSANOW, &original_settings);
  dwebserver_kill();
  kill_player();
  if (err != NULL)
  {
    fclose(err);
  }
  
  pthread_cancel(server_thread_id);
  pthread_join(server_thread_id, NULL);
  puts("Bye");
}

void wait_for_key()
{
  struct termios unbuffered;
  tcgetattr(STDIN_FILENO, &original_settings);

  unbuffered = original_settings;
  unbuffered.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSANOW, &unbuffered);

  getchar();
  close_down();
}

int set_vol(int volume)
{
  double log_vol;
  
  if (volume > 0)
  {
    #ifdef __arm__
      log_vol = (40 * log10((double)volume)) + 48;
    #elif __OPENWRT__
      log_vol = volume * 1.5; // linear
    #else
      log_vol = (60 * log10((double)volume)) + 22;
    #endif
  }
  else
  {
    log_vol = 0;
  }

  char num[5];
  snprintf(num, 5, "%d%%", (int)log_vol);
  //printf("%d = %s\n", volume, num);
  char *mixer[] = { "/usr/bin/amixer", "-q", "sset", AUDIO, num, 0};
  return run(mixer) > 0 ? 0 : 1;
}

void kill_player()
{
  if (lastplayerpid > 0)
  {
    kill(lastplayerpid, SIGKILL);
  }
  lastplayerpid = 0;

  if (lastwgetpid > 0)
  {
    kill(lastwgetpid, SIGKILL);
  }
  lastwgetpid = 0;
}

int main(int argc, char **argv)
{
  if (argc < 2 || !strncmp(argv[1], "-h", 2))
  {
    printf("hint: radioApi [port number]\n");
    return 0;
  }
  
  // anything going to stderr will get sent to "errors.txt"
  err = fopen("errors.txt", "a");
  if (err != NULL)
  {  
    dup2(fileno(err), STDERR_FILENO);
  }
  set_vol(vol);
  
  if (argc > 2 && !strncmp(argv[2], "-d", 2))
  {
    // don't read from the console or log anything
    dwebserver(atoi(argv[1]), &send_response, &null_log);
  }
  else
  {
    if (pthread_create(&server_thread_id, NULL, server_thread, argv[1]) != 0)
    {
      fputs("ERROR: pthread_create could not create server thread", stderr);
      return 0;
    }

    puts("Radio server started\nPress any key to quit.");
    wait_for_key();
  }
  
  return 0;
}

void log_filter(log_type type, char *s1, char *s2, int socket_fd)
{
  if (type != ERROR)
  {
    return;
  }
  fprintf(stderr, "ERROR: %s: %s (errno=%d pid=%d socket=%d)\n",s1, s2, errno, getpid(), socket_fd);
}

void null_log(log_type type, char *s1, char *s2, int socket_fd)
{
  // don't do anything...
}

// decide if we need to send an API response or a file...
void send_response(struct hitArgs *args, char *path, char *request_body, http_verb type)
{
  int path_length=(int)strlen(path);
    
  if (!strncmp(&path[path_length-3], "api", 3))
  {
    return send_api_response(args, path, request_body);
  }

  if (path_length==0)
  {
    return send_file_response(args, "index.html", request_body, 10);
  }
    
  send_file_response(args, path, request_body, path_length);
}

void send_api_response(struct hitArgs *args, char *path, char *request_body)
{
  int error = 0;
  
  if (args->form_value_counter > 0 && string_matches_value(args->content_type, "application/x-www-form-urlencoded"))
  {
    int v;
    for (v=0; v<args->form_value_counter; v++)
    {
      if (strncmp("streamurl", form_name(args, v), 9) == 0)
      {
	kill_player();
	
	if (strlen(form_value(args, v)) > 0 && strncmp("stop", form_value(args, v), 4) != 0)
	{
	  // play the stream
	  char *wget[] = { "/usr/bin/wget", "-q", "-O", "-", form_value(args, v), 0 };
	  char *player[] = { "/usr/bin/madplay", "-Q", "--fade-in=0:01", ADJUST, "-", 0 };
	  execpiped(wget, player, &lastwgetpid, &lastplayerpid);
	  if (lastwgetpid == 0 || lastplayerpid == 0)
	  {
	    error = 1;
	  }
	}
      }
      else if (strncmp("volup", form_name(args, v), 5) == 0)
      {
	// increase volume
	vol += vol >= 20 ? 0 : 1;
	error = set_vol(vol);
      }
      else if (strncmp("voldn", form_name(args, v), 5) == 0)
      {
	// decrease volume
	vol -= vol <= 0 ? 0 : 1;
	error = set_vol(vol);
      }
      else if (strncmp("volume", form_name(args, v), 6) == 0)
      {
	// set volume
	vol = atoi(form_value(args, v));
	if (vol > 20)
	{
	  vol = 20;
	}
	if (vol < 0)
	{
	  vol = 0;
	}
	error = set_vol(vol);
      }
    }

    if (error == 1)
    {
      write_html(args->socketfd, "HTTP/1.1 409 Conflict\nContent-Type: text/plain\nConnection: close", "ERROR\n");
    }
    else
    {
      ok_200(args, "\nConnection: close\nContent-Type: text/plain", "OK\n", path);
    }
  }
  else
  {
    forbidden_403(args, "Bad request\n");
  }
}

void send_file_response(struct hitArgs *args, char *path, char *request_body, int path_length)
{
  int file_id, i;
  long len;
  char *content_type = NULL;
  STRING *response = new_string(FILE_CHUNK_SIZE);

  // work out the file type and check we support it
  for (i = 0; extensions[i].ext != 0; i++)
  {
    len = strlen(extensions[i].ext);
    if (!strncmp(&path[path_length-len], extensions[i].ext, len))
    {
      content_type = extensions[i].filetype;
      break;
    }
  }
  
  if (content_type == NULL)
  {
    string_free(response);
    return forbidden_403(args, "file extension type not supported");
  }
	
  if (file_id = open(path, O_RDONLY), file_id == -1)
  {
    string_free(response);
    return notfound_404(args, "failed to open file");
  }
	
  // open the file for reading
  len = (long)lseek(file_id, (off_t)0, SEEK_END); // lseek to the file end to find the length
  lseek(file_id, (off_t)0, SEEK_SET); // lseek back to the file start
    
  if (len > BIGGEST_FILE)
  {
    string_free(response);
    return forbidden_403(args, "files this large are not supported");
  }
    
  string_add(response, "HTTP/1.1 200 OK\nServer: dweb\n");
  string_add(response, "Connection: close\n");
  string_add(response, "Content-Type: ");
  string_add(response, content_type);
  write_header(args->socketfd, string_chars(response), len);
    
  // send file in blocks
  while ((len = read(file_id, response->ptr, FILE_CHUNK_SIZE)) > 0)
  {
    if (write(args->socketfd, response->ptr, len) <=0)
    {
      break;
    }
  }
  string_free(response);
  close(file_id);
}

// pipe the output of the first command to the second
void execpiped(char **cmdfrom, char **cmdto, int *frompid, int *topid)
{
  int pipefd[2];
  pipe(pipefd);

  *frompid = fork();
  if (*frompid == 0)
  {
    // see: http://stackoverflow.com/questions/2605130/redirecting-exec-output-to-a-buffer-or-file
    // this command sends stdout to the pipe
    signal(SIGPIPE, SIG_DFL);
    close(pipefd[0]);   // close reading end in child process
    dup2(pipefd[1], 1); // send stdout
    close(pipefd[1]);   // no longer needed
    execv(cmdfrom[0], cmdfrom);
    exit(1);
  }
  
  close(pipefd[1]); // close write end in parent
  
  if (*frompid <= 0)
  {
    // didn't start the first command...
    *frompid = 0;
    return;
  }
  
  *topid = fork();
  if (*topid == 0)
  {
    // see: http://stackoverflow.com/questions/9487695/redirecting-input-from-file-to-exec
    // this command reads stdin from the stdout of the last command
    signal(SIGPIPE, SIG_DFL);
    dup2(pipefd[0], 0); // get stdin
    close(pipefd[0]);   // no longer needed
    execv(cmdto[0], cmdto);
    exit(1);
  }
  
  close(pipefd[0]); // close read end in parent
  
  if (*topid <= 0)
  {
    // didn't start the second command...
     kill(*frompid, SIGKILL);
     *frompid = 0;
     *topid = 0;
     return;
  }
}

int run(char **cmd)
{
  int pid = fork();
  if (pid == 0)
  {
    execv(cmd[0], cmd);
    exit(1);
  }
  return pid;
}
