## test 6: lazy binding
Previous tests are all implemented as instant binding, that is, before the loading ends,
all the relocations are resolved. This is quite time-consuming, because we may look up the symbols
that are never used by our program.

So here goes lazy binding, a technique to delay relocation to the time a function is called.
Normally, the relocation of a function finds the real address and fills it into the PLT entry,
so that the function will be called when the control flow goes to that entry.

Lazy binding avoids directly call that function. Instead, it uses a special function implemented 
by the loader to resolve the actual address.
<!-- TODO: maybe a commented version of the illustration 7-19? -->
Let's see the real code in PLT using `objdump -d ./test_lib/SimpleDep.so`:
```
Disassembly of section .plt:

0000000000001020 <.plt>:
    1020:       ff 35 e2 2f 00 00       pushq  0x2fe2(%rip)        # 4008 <_GLOBAL_OFFSET_TABLE_+0x8>
    1026:       ff 25 e4 2f 00 00       jmpq   *0x2fe4(%rip)        # 4010 <_GLOBAL_OFFSET_TABLE_+0x10>
    102c:       0f 1f 40 00             nopl   0x0(%rax)

0000000000001030 <multiply@plt>:
    1030:       ff 25 e2 2f 00 00       jmpq   *0x2fe2(%rip)        # 4018 <multiply>
    1036:       68 00 00 00 00          pushq  $0x0
    103b:       e9 e0 ff ff ff          jmpq   1020 <.plt>
```
And to see what in original inside PLT using `objdump -s ./test_lib/SimpleDep.so | grep 4010`:
```
4010 00000000 00000000 36100000 00000000  ........6.......
```
Things are pretty clear now: When you first call multiply, the control flow jumps to `0x4018` as shown
in `0x1030`. And this makes what lies at `0x4018` important. If you decide to use instant binding,
then you should fill the actual address of `multiply`, and the control flow will go normally.

However, if you want to perform lazy binding, you will want the control flow to jump to `0x1036` becasue
it will push the index of relocation entry, and jump to `0x1020`, where `GOT[1]` gets pushed, and the 
control flow jumps to the address in `GOT[2]`. As a result, we should first process relocation entry
of `multiply` in a 'relative' way, simply adding existing value with the base address of 
this library (as in test 2).

Before calling the function at `GOT[2]` in `0x1026`, 2 values have been pushed into the stack.
So this function should fetch the values from the stack and place them into `rdi` and `rsi`, which
store the 1st and 2nd argument for a function call according to x86-64 calling convention, 
and call another function to resolve the address of the current symbol.

In my implementation, `GOT[2]` is filled with the address of function `trampoline`, `trampoline`
calls `runtimeResolve` to perform the actual relocation, and fill in the PLT entry with the real 
address. `trampoline` then jumps to the address found, and the lazy relocation is done.

In the second call to `0x1030`, `0x4018` now contains the actual address, and `multiply` will be called
directly instead of `trampoline`.
