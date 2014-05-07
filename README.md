# osxinj_tut

This project demonstrates and documents how to use `mach_inject` and
`mach_override` to hijack a function within a running process. See
`tutorial.md` for details.

# Compiling

This project requires **XCode** (and the command-line build tools, if you
prefer `make` like I do) and **CMake**. I'm not a particular fan of using XCode
(too much noise, IMO) but I realize other people are. I went with CMake so I
could have it both ways.

Here's how I build:

1. `cd` into the `osxjin_tut/` folder
2. Run `cmake -G "Unix Makefiles"; make`

I haven't done it, but if you want an XCode project, you should be able to do

    cmake -G "XCode"

and be back in your comfort zone.
