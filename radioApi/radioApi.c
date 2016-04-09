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

#include "dwebsvr.h"

#define FILE_CHUNK_SIZE 1024
#define BIGGEST_FILE 104857600 // 100 Mb

#ifdef __arm__
  #define AUDIO "PCM"
  #define ADJUST "-a0"
#elif __OPENWRT__
  #define AUDIO "Headphone"
  #define ADJUST "-a-6"
#else
  #define AUDIO "Master"
  #define ADJUST "-a12"
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

void quit_handler(int signal);
int set_vol(int volume);
void send_response(struct hitArgs *args, char*, char*, http_verb);
void log_filter(log_type, char*, char*, int);
//void kill_player();
void kill_all_children();
void send_api_response(struct hitArgs *args, char*, char*);
void send_file_response(struct hitArgs *args, char*, char*, int);
void execpiped(char **cmdfrom, char **cmdto, int *frompid, int *topid);
int run(char **cmd);

struct termios original_settings;
pthread_t server_thread_id;
int volpc = 25, lastwgetpid = 0, lastplayerpid = 0;
pid_t parent_pid;

void quit_handler(int signal)
{
  if (signal != SIGQUIT)
  {
    return;
  }
  pid_t current = getpid();
  if (parent_pid != current)
  {
    exit(0);
  }
}

void* server_thread(void *args)
{
  pthread_detach(pthread_self());
  char *arg = (char*)args;
  dwebserver(atoi(arg), &send_response, &log_filter);
  return NULL;
}

void close_down()
{
  tcsetattr(STDIN_FILENO, TCSANOW, &original_settings);
  dwebserver_kill();
  kill_all_children();
  pthread_cancel(server_thread_id);
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
  char num[5];
  snprintf(num, 5, "%d%%", volume);
  char *mixer[] = { "/usr/bin/amixer", "-q", "sset", AUDIO, num, "-M", 0};
  return run(mixer) > 0 ? 1 : 0;
}

void kill_all_children()
{
  // quit all children
  kill(-parent_pid, SIGQUIT);
  lastwgetpid = 0;
  lastplayerpid = 0;
}

/*void kill_player()
{
  if (lastplayerpid !=0)
  {
    kill(lastplayerpid, SIGKILL);
    lastplayerpid = 0;
  }
	
  if (lastwgetpid !=0)
  {
    kill(lastwgetpid, SIGKILL);
    lastwgetpid = 0;
  }
}*/

int main(int argc, char **argv)
{
  if (argc < 2 || !strncmp(argv[1], "-h", 2))
  {
    printf("hint: radioApi [port number]\n");
    return 0;
  }
  
  signal(SIGQUIT, quit_handler);
  parent_pid = getpid();
  
  set_vol(volpc);
  
  if (argc > 2 && !strncmp(argv[2], "-d", 2))
  {
    // don't read from the console or log anything
    dwebserver(atoi(argv[1]), &send_response, NULL);
  }
  else
  {
    if (pthread_create(&server_thread_id, NULL, server_thread, argv[1]) != 0)
    {
      puts("Error: pthread_create could not create server thread");
      return 0;
    }

    puts("Radio server started\nPress any key to quit.");
    wait_for_key();
  }
}

void log_filter(log_type type, char *s1, char *s2, int socket_fd)
{
  if (type != ERROR)
  {
    return;
  }
  printf("ERROR: %s: %s (errno=%d pid=%d socket=%d)\n",s1, s2, errno, getpid(), socket_fd);
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

int adjust_volume(int vol)
{
  return set_vol(vol) == 1 ? 0 : 1;
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
	kill_all_children();
	
	if (strlen(form_value(args, v)) > 0 && strncmp("stop", form_value(args, v), 4) != 0)
	{
	  // play the stream
	  char *wget[] = { "/usr/bin/wget", "-q", "-O", "-", form_value(args, v), 0 };
	  char *player[] = { "/usr/bin/madplay", "-Q", "--fade-in=0:01", ADJUST, "-", 0 };
	  execpiped(wget, player, &lastwgetpid, &lastplayerpid);
	  if (lastwgetpid <= 0 || lastplayerpid <= 0)
	  {
	    error = 1;
	  }
	}
      }
      else if (strncmp("volup", form_name(args, v), 5) == 0)
      {
	// increase volume
	volpc += volpc>=100 ? 0 : 20;
	error = adjust_volume(volpc);
      }
      else if (strncmp("voldn", form_name(args, v), 5) == 0)
      {
	// decrease volume
	volpc -= volpc<=0 ? 0 : 20;
	error = adjust_volume(volpc);
      }
      else if (strncmp("volume", form_name(args, v), 6) == 0)
      {
	// set volume
	volpc = 5 * atoi(form_value(args, v));
	error = adjust_volume(volpc);
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
  for (i=0; extensions[i].ext != 0; i++)
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
    close(pipefd[0]); // close reading end in child process
    dup2(pipefd[1], 1); // send stdout
    close(pipefd[1]); // no longer needed
    execv(cmdfrom[0], cmdfrom);
    exit(1);
  }
  
  sleep(1); // buffer slightly before playing
  
  close(pipefd[1]); // close write end in parent
  
  *topid = fork();
  if (*topid == 0)
  {
    // see: http://stackoverflow.com/questions/9487695/redirecting-input-from-file-to-exec
    // this command reads stdin from the stdout of the last command
    dup2(pipefd[0], 0); // get stdin
    close(pipefd[0]); // no longer needed
    execv(cmdto[0], cmdto);
    exit(1);
  }
  
  close(pipefd[0]); // close read end in parent
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
