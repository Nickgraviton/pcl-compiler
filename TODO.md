##  TODO:
- Add semantic pass
- Add symbol table (hash table implementation)
- Add code generation
- Add optimization passes

## Outline:
- Complete AST adding all necessary nodes and create a tree while parsing
- Make a semantic pass on the tree we created keeping info about types and checking for valid conversions, assignments etc
- Add final code generation pass using llvm
