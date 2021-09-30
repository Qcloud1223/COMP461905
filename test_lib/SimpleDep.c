// depend on SimpleMul
// I haven't decided whether to make rpath handling a testcase,
// but if I choose not, I can use '.' as a default library to bypass
extern int multiply(int, int);
int wrapper(int a, int b)
{
    return multiply(a, b);
}