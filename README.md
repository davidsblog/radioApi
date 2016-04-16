radioApi
====

A small Web-API for playing internet radio streams using **madplay**.  I am mainly using it on a tiny router running OpenWrt, but 
I do use it on a Raspberry Pi as well.  I'm using **madplay** because it runs nicely on my router, and it streams internet radio pretty well. 
*These* build instructions are aimed at Debian-based systems.  But cross-compiling for OpenWrt is also possible.

NOTE: I'm currently testing it on a **hardened** version of Raspbian, aka 
[IPE V1 R2](http://www.andreasgiemza.de/allgemein/ipe-r1-v2/), and it needs me to run ```modprobe snd_bcm2835``` 
to enable sound output before I start the server.  The hardened version of Raspbian is useful because it uses a 
read-only file system, so I can safely switch the Raspberry Pi off at any time without killing file system on my SD card. 
It also means that I can run my Radio Player on Raspberry Pi from a battery.

I have included a simple UI which I'm using in the browser of my Android phone to work as a remote control, it looks like this: 

![User interface screenshot](interface.png?raw=true "User interface")

To see details of the API, build and run the code and use the `API` link from the top menu to view `/doc.html` in your browser.

At the moment, I am redirecting any errors (output going to `stderr`) to a file called **errors.txt** and you should be able to 
view that in your browser (ie http://localhost:8112/errors.txt) in case there are problems.  In theory you should wipe this file 
from time to time to prevent it from growing too big.

**NOTE: the code is really designed for using the _headphone socket_ as the audio output.**

Prerequisites
====

You will need **wget** to read the streams, but it should be installed already.  It's best to do an update first:
```
sudo apt-get update
```

And you'll need **madplay** to play the radio streams:
```
sudo apt-get install madplay
```

You'll need **alsa-utils** for the volume control feature:
```
sudo apt-get install alsa-utils
```

Please test these packages are working by doing:
```
madplay --help
amixer --help
```

...to make sure they display their help texts, the code won't work without them.

###How to build and run

#####On Debian based systems (including the Raspberry Pi)

You should be able to do this:
```
sudo apt-get install git
git clone https://github.com/davidsblog/radioApi
cd radioApi/radioApi/
sudo make install
```

...which will build everything and install it as a service (it will run at system start-up).  **The server will run on port 8112.** 
That means you can view the player by visiting http://localhost:8112/. 
You can remove it from your system like this (assuming you are still in the `radioApi/radioApi/` directory):
```
sudo make uninstall
```

**NOTE:** the `sudo` before calling make above is important, since you're installing services.

#####Runing manually (or on different Linux versions)
Just do this:
```
sudo apt-get install git
git clone https://github.com/davidsblog/radioApi
cd radioApi/radioApi/
make
./radioApi 80
```

You might have a webserver already running on port 80, in which case you can specify a different port by passing a different parameter than **80** in the last line above.

License
=======

The MIT License (MIT)

Copyright (c) 2016 David's Blog - www.codehosting.net

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
