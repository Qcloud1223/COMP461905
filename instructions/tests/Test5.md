## test 5: deal with dependency tree
If you are careful enough, you should have found it in test 3: What if the dependencies of the 
dependencies? `DT_NEEDED` only records the direct dependencies, that is, a library will only 
appear as a `DT_NEEDED` of another library if its function is referred by that library.

That means introducing a dependency will introduce new dependencies. Finally, opening a library
will introduce a dependency tree, and you need to use BFS to map it all.