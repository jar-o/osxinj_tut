#include <cstdio>
#include <unistd.h>

/*
    Our default print function. This prints in a loop until the program exits.
    This will get hijacked by the code in testlib.dylib via osxinj.

    Note, since we're compiling with C++ flags, you need to make sure that the

        extern "C"

    is in place, or print_hw() will get the C++ name decorations, which will
    prevent testlib.dylib code from finding this function to override it. As a
    'C' function we know it will be named '_print_hw' in the binary.
*/
extern "C" void print_hw()
{
    printf("testapp: helowrld\n");
}

/*
    This will run for a long time, giving us the chance to execute our
    injection code form the command line.
*/
int main(int argc, char* argv[])
{
    printf("testapp reporting in!\n");
    int i = 0;
    while( i < 1000)
    {
        print_hw();
        sleep(1);
        i++;
    }
    return 0;
}
