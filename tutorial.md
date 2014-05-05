# OSX injection tutorial: Hello World

<sub>2014-05-04, James Robson, <http://soundly.me></sub>

This is a *Hello World* tutorial that demonstrates (1) injecting code into a
running process, and (2) overriding a function in that process.

It's based on the project `osxinj` by Stanley Cen, which you can find here:

<https://github.com/scen/osxinj>

That project was in turn based on `mach_inject` created by Jonathon Rentzsch.

<https://github.com/rentzsch/mach_inject>

The `osxinj` project took the `mach_inject` code and made a nice and clear
sample out of it. It was Xcode based, and easy to get working. The only things
I found lacking were the ability to override a specific function after
injection (the `mach_override` stuff), and of course a nice tutorial tying all
the parts together. That's what this tries to do.

## The code

There are four source directories in this project, each a separate compilable
unit, laid out pretty much the same as `osxinj` (except that we've moved them
under a `src/` folder for compiling reasons:

    src/
        bootstrap/
        osxinj/
        testapp/
        testdylib/

The first two, `bootstrap/` and `osxinj/` are basically untouched. This tutorial
isn't about them, but getting a basic understanding of how they work will be
valuable to you.

`osxinj/` compiles to a utility that allows you to inject a libary (a `.dylib`
file) into a running process. Here's the usage message:

    Usage: osxinj [proc_name] [lib]

(Note, you'll generally need to `sudo` to get it to work.)

The last two we're going to talk about. `testapp` is a very simple application
that just loops for 1,000 seconds and prints "helowrld" to STDOUT.  `testdylib`
is where we'll do our injection into `testapp` and hijack of one of its
functions. In this case, `testapp` is the victim, and `testdylib` is the
predator. ;)

## The victim

There is very little that's interesting about `testapp`. It just loops and
calls a function `print_hw()` that prints a line to the console. In fact,
here's the function in its entirety:

    extern "C" void print_hw()
    {
        printf("testapp: helowrld\n");
    }

The whole point of `testapp` is to be an easy target. It just starts up and
every second calls `print_hw()`. This way we have time to start it, then run
`osxinj` on `testlib.dylib` and watch the function get hijacked.

The one thing of note is the `extern "C"` declaration above. We have to
discover this function in the running binary, and we use the
`_dyld_lookup_and_bind()` call, which allows us to pass in a string identifying
the function we want. If the function is found, we'll get a pointer to it.
Compiled C functions are always prefixed by `_` and are otherwise unmolested --
unlike C++ name mangling. Using this declaration means we can pass
`"_print_hw"` and be fairly confident we'll get a pointer.

## The predator

The `testdylib` code is where the fun begins. It contains the following declarations:

    long *victim_func_ptr;
    void (*victim_func)() = 0;

`victim_func_ptr` is where we'll store the address of the `print_hw()` function
we discover in the running `testapp` process. `victim_func` is a function
pointer to which we'll cast `victim_func_ptr`. It will allow us to call
`print_hw()` from our override function. Meaning, we don't have to lose the
functionality of the original function we hijack.

Next, we have our override function:

    void my_print_hw()
    {
        printf("testlib: heloWRLD\n");
        (*victim_func)();
    }

First, it prints out a message letting us know it's here and in charge. Then,
it calls the hijacked function `print_hw()` via the function pointer.

There is only one remaining function, `install()`. It does both the injecting
and the overriding.

    void install(void) __attribute__ ((constructor));
    void install()
    {
        printf("testlib: install\n");

        _dyld_lookup_and_bind(
            "_print_hw",
            (void**) &victim_func_ptr,
            NULL);

        victim_func = (void (*)())victim_func_ptr;

        mach_error_t me;
        me = mach_override_ptr(
                          victim_func_ptr,
                          (void*)&my_print_hw,
                          (void**)&victim_func);

    }

Note that the name of the function is irrelevant. What is relevant is the line

    void install(void) __attribute__ ((constructor));

which is a signature that informs the injection code that this function will
get inserted and executed in the external process.

Once the code is injected and running in the target process, the following code
is called with the string `"_print_hw"` which is the compiled "C" name of the
function we want to override:

        _dyld_lookup_and_bind(
            "_print_hw",
            (void**) &victim_func_ptr,
            NULL);

If it's found in the process, `victim_func_ptr` will hold the address to the
"active" (that is, in memory) `print_hw()` function.

To use it we have to assign it to our function pointer, using some frightening
casting syntax. I'm not going to go into that (ugh) -- suffice it to say that
this allows us to call the function that's pointed to by `victim_func_ptr`.

        victim_func = (void (*)())victim_func_ptr;

Remember the second line of `my_print_hw()` above? It had to point at
something, and this is how we "learn" what to point at during runtime.

Last, and certainly not least, we override the `print_hw()` function, passing
it the pointers we've setup above. Note we have to pass in the `victim_func`
pointer as the third argument since we plan to call `print_hw()` from our code.
You can leave that parameter `NULL` if you're not going to use the original
function at all.

        mach_error_t me;
        me = mach_override_ptr(
                          victim_func_ptr,
                          (void*)&my_print_hw,
                          (void**)&victim_func);

Ideally, you would check `me` for errors. But if all goes well, at this point
we control the `print_hw()` function in a live running process.

## Testing it out

After the code is compiled, you run it as follows. (Note, we are expecting
you to be in the root of this project folder when you run these.)

In one Terminal, start `testapp`:

    ./testapp
    testapp reporting in!
    testapp: helowrld
    testapp: helowrld
    ...

It will do that for 1,000 seconds, so you have plenty of time. Now, in another
Terminal do the following command:

    sudo ./osxinj testapp testdylib.dylib

Now, if you look back in the first Terminal window you should see the following:

    ./testapp
    testapp reporting in!
    testapp: helowrld
    testapp: helowrld
    testapp: helowrld
    testapp: helowrld
    testapp: helowrld
    testapp: helowrld
    testapp: helowrld
    testlib: install
    testlib: victim_func_ptr   = 278160
    testlib: victim_func       = 278160
    testlib: heloWRLD
    testapp: helowrld
    testlib: heloWRLD
    testapp: helowrld
    testlib: heloWRLD
    testapp: helowrld
    testlib: heloWRLD
    testapp: helowrld
    ...

`testapp` is still running, and still naively thinks that it's calling
`print_hw()`. However, you can see that `my_print_hw()` has taken over.
