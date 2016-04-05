radioApi
====

A small web API for playing internet radio using **madplay**.  I am currently using it on a router running OpenWrt, but 
I will use it on a Raspberry Pi too at some point.

To see details of the API, build and run the code and use the API link from the top menu to view `/doc.html` in your browser.

Prerequisites
====

You will need **wget** to read the streams, but it should be installed already.

You'll need **madplay** to play the files:

```
sudo apt-get install madplay
```

You'll need **alsa-utils** for the volume control feature:

```
sudo apt-get install alsa-utils
```

Test these by doing:

```
madplay --help
amixer --help
```

To make sure they display the help.

Building
========

On most Debian based systems you should be able to do this:

```
git clone https://github.com/davidsblog/radioApi.git
cd radioApi/radioApi/
make
```

Running
=======

After building from source, as long as you are in the `radioApi/radioApi` directory, you can do this:

```
./radioApi 8112
```

..which will run the server on port `8112` and so you can point your browser to http://localhost:8112 and try it out.

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
