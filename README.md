tree-awesome
============

Arduino code to control stuff with a built-in webserver

## Requirements

This program needs the library Ethercard to be included in your project (see https://github.com/jcw/ethercard)
as it is using the enc28j60 Ethernet module.

## What is this?

This Arduino program was imagined to control power outlets from a webserver. 

Find out more at http://florianfries.me/en/a-connected-christmas.html


## Can I use this?

Probably, but the modes and stuff where thought to be used in my environment 
(to light up a Christmas tree) so you will have to adapt it.

But some things are interesting in the program:
- Launching a web server
- Returning json data when a request is made
- Analyzing the url of the HTTP request to perform actions relative to it
