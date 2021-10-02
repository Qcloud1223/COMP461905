// dependency with an additional layer
// This is to make sure that you've done relocation for
// the whole dependency tree of the library being opened
extern int wrapper(int, int);
int wrapperAgain(int a, int b)
{
    return wrapper(a, b);
}
