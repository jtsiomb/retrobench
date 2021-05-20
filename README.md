Retrobench
==========
A multi-platform framebuffer drawing performance benchmark targetted towards
retro PCs.

Retrobench intends to test how different operating systems, drawing methods,
and graphics hardware, affect drawing performance for a pure software rendering,
framebuffer access context.

Currently implemented benchmarks:

  - Fast XOR pattern scrolling: pure framebuffer write speed testing

Currently supported platforms/drawing methods:

  - UNIX X11 with the X shared memory extension (XShmPutImage).
  - GNU/Linux with framebuffer console (fbdev).
  - DOS direct framebuffer access through VESA BIOS Extensions (VBE).

Read about why I started this, and my initial findings:
http://nuclear.mutantstargoat.com/blog/100-fbperf_experiments.html

License
-------
Copyright (C) 2021 John Tsiombikas <nuclear@member.fsf.org>

This program is free software. Feel free to use, modify, and/or redistribute it
under the terms of the GNU General Public License v3, or at your option any
later version published by the Free Software Foundation. See COPYING for
details.

Build on UNIX
-------------
Simply typing `make` should build both the X11 and fbdev version of the
benchmark. The only dependency is Xlib, for the X11 version.

Build on DOS
------------
Set up a DJGPP build environment and type `make -f Makefile.dj`.
