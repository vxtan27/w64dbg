#include <debugapi.h>

struct Example
{
    //Infinite recursion
    void RootException(void)
    {
        CauseException();
    }
    //Start the recursion
    void CauseException(void)
    {
        RootException();
    }
};

int main(void)
{
    Example example;
    DebugBreak();
    example.CauseException();
}