# RPAL-Interpreter
Implement a lexical analyzer and a parser for the RPAL language. Output of the parser gives the Abstract Syntax Tree (AST) for the given input program. Implemented an algorithm to convert the Abstract Syntax Tree (AST) in to Standardize Tree (ST) and implement CSE machine.


before run this compile this using following command

> g++ rpal-interpreter.cpp lexicon.cpp psg.cpp asttost.cpp flattenst.cpp cse.cpp -o rpal.exe

then use following command to run the rpal.exe

> ./rpal.exe