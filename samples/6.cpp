#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>

class Example
{
public:
    void CauseException(void)
    {
        RootException();
    }
private:
    static inline void RootException(void)
    {
        // Cause breakpoint
        DebugBreak();
    }
};

int main(void) {
    Example example;
    example.CauseException();
}