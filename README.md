radioApi
====

##Work in progress...

A small Web-API for playing internet radio streams using **madplay**.  I am currently using it on a router running OpenWrt, but 
I will use it on a Raspberry Pi too at some point.  *These* build instructions are aimed at Debian-based systems.

I have included a simple web-based UI which I'm using in the browser of my Android phone to work as a remote control. 

To see details of the API, build and run the code and use the `API` link from the top menu to view `/doc.html` in your browser.

**NOTE: the code is set for using the _headphone socket_ as the audio output.**

Prerequisites
====

You will need **wget** to read the streams, but it should be installed already.

You'll need **madplay** to play the radio streams:

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

#####On Debian based systems (and the Raspberry Pi probaby)
You should be able to do this:
```
git clone https://github.com/davidsblog/radioApi
cd radioApi/radioApi/
sudo make install
```
...which will build everything and install it as a service (it will run at system start-up).  **The server will run on port 8112.** That means you can view the player by visiting http://192.168.1.2:8112/ (you need to substitute your machine's IP address). You can remove it from your system like this (assuming you are still in the `radioApi/radioApi/` directory):
```
sudo make uninstall
```

**NOTE:** the `sudo` before calling make above is important, since you're installing services.

#####Runing manually (or on different Linux versions)
Just do this:

```
git clone https://github.com/davidsblog/radioApi
cd radioApi/radioApi/
make
sudo ./radioApi 80
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
