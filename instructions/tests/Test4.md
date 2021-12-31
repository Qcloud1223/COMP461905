## test 4: relocate global variables
<!-- TODO: check if this is too easy -->
Now we talk about the last relocation type in this project: `R_X86_64_GLOB_DAT`. 
There is no big difference between this one and the previous two, so details will be excluded.

The relocation entries for global variables are also in `.rela.dyn`, mixed with relative entries.
The way to locate it is using `DT_RELACOUNT`, the number of relative relocations.
`DT_RELASZ` gives the total size of the `.rela.dyn`, and the number of non-relative relocations 
can be calculated from them.

Note that `DT_RELACOUNT` is a GNU extension (defined to `0x0x6ffffff9`), 
and in this project I define `DT_RELACOUNT_NEW` to `DT_NUM` (34), 
so that it can be directly stored in the `dynInfo` field of `LinkMap`.
You can check this behavior in `fill_info` at `src/MapLibrary.c`.

The address to be filled in relocation entry can also be returned by `findSymbol`, identical to 
`R_X86_64_JUMP_SLOT`.
