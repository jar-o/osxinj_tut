#include <cstdio>

#include "mach_override.h"
#include "mach-o/dyld.h"
#include <CoreServices/CoreServices.h>




// Used to track the pointer to victim code's function
long *victim_func_ptr;


// A function prototype so we can call the victim function from our override
// function.
void (*victim_func)() = 0;


// Our override function
void my_print_hw()
{
    // Our hijacked code ...
    printf("testlib: heloWRLD\n");

    // Optionally, we can still call the original function, which is useful and
    // even necessary in some case ...
    (*victim_func)();
}


void install(void) __attribute__ ((constructor));
void install()
{
    printf("testlib: install\n");

    // Use this to discover a pointer to the function we want to hijack
    _dyld_lookup_and_bind(
        "_print_hw",
        (void**) &victim_func_ptr,
        NULL);

    //TODO check for bad victim_func_ptr

    // Assign our long pointer to our function prototype
    victim_func = (void (*)())victim_func_ptr;

    printf("testlib: victim_func_ptr   = %ld\n", (long)victim_func_ptr);
    printf("testlib: victim_func       = %ld\n", (long)victim_func);

    // Do the override
    mach_error_t me;

    me = mach_override_ptr(
                      victim_func_ptr,
                      (void*)&my_print_hw,
                      (void**)&victim_func);

}
