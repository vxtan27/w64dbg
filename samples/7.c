#include <stddef.h>

int main(void)
{
    typedef void(*CallbackFunc)(void);
    CallbackFunc invalidCallback = NULL;
    invalidCallback(); // Dereference a null function pointer
}