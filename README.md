# Kaleidoscope

[![LLVM](https://img.shields.io/badge/LLVM-9.0.1-blue)](https://llvm.org/)

[Kaleidoscope](https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html) is a toy programming language built from scratch using the `LLVM` libraries.

The code in this repo is taken from the first 3 chapters of the excellent tutorial about `LLVM`: [My First Language Frontend with LLVM Tutorial](https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl01.html#the-kaleidoscope-language).

I copied the first chapters tutorial's code with some personal additions:
- implemented the unittests
- adopted an object oriented approach to replace the global static variables with instance variables
- used the `Visitor` design pattern to generate the IR code

The goal of this repo is to learn more about programming languages, LLVM and to renew my C++ skills.

* [Programming Languages Concepts](#programming-languages-concepts)
* [Introduction To LLVM](#introduction-to-LLVM)
* [About The Code](#about-the-code)
* [Code Style](#code-style)
* [Acknowledgements](#acknowledgements)


## Programming Languages Concepts
TODO

### Lexer

It is the first phase of the compiler's frontend.
It consists of converting a sequence of characters into a sequence of tokens.
A token is a string with an assigned and thus identified meaning.

Common token names are:

- identifier: names the programmer chooses;
- keyword: names already in the programming language;
- separator (also known as punctuators): punctuation characters and paired-delimiters;
- operator: symbols that operate on arguments and produce results;
- literal: numeric, logical, textual, reference literals;
- comment: line, block.


## Introduction to LLVM

The `LLVM Project` is a collection of modular and reusable compiler and toolchain technologies, born in December 2000.

The primary sub-projects of LLVM are:

- The LLVM Core libraries (partially used in this repo)
- Clang: C/C++/Obj-C compiler with an advanced error and warning analysis
- LLDB: the Clang debugger

At the time the `LLVM` project born the main compilation and interpreter tools were conceived as monolithic block,
making it difficult to reuse only some part of the compilation toolchain (for example just the parser of the `GCC` to do some static analysis or refactoring).

In addition to this the compiled or interpreted languages were implemented with either static compilers or JIT interpreters,
and it did not exists a toolchain to use both of them in a clear and modular way.

`LLVM` uses a modular 3 phase compilation process.

The `GCC` uses the 3 phase compilation process, but it does uses a modular approach that allows to use one piece without dealing with the other pieces.
This is due to the fact that in `GCC`:
- there is a rampant use of global variables, define clauses
- the back-end walks front-end ASTs to generate debug info
- the front ends generate back-end data structures
- the entire compiler depends on global data structures set up by the command line interface

Even if `GCC` was great at implementing several front-ends and several back-ends by a big and active community,
the resulting compilers where monolithic blocks.

The `Java Virtual Machine` use a 3 phase approach.

The bytecode generated is well defined so that it is possible to implement your own frontend while keeping the same backend.
However it is the backend with the `JVM` run time environment that is coupled with the peculiarities of the Java language in terms of:
- forcing the JIT compilation
- garbage collection
- very particular object model

and makes it inefficient when compiling languages that don't match this model closely, such as C.

### What it JIT

Programming languages such as C are statically compiled, meaning that the source code is converted by a toolchain (such as GCC or Clang) to executable machine code in one pass.
One pass means that once the machine code is generated once and can be executed multiple times.

Static compilation is used to obtain efficient code at the expense of code portability.

In fact the machine code generated is targeted for a specific ABI and binary compatibility is ensured only if the platforms implements the same ABI.

This means when the platform to run the code change, it could be required to re-compile the source file.
In addition to this the compilation process can be time-consuming.

Interpreted languages generally follow the same approach:
- the source code is converted into bytecode that is platform independent
- the interpreter executes the bytecode at run time, compiling to native code and executing it.

To make things faster after the code is converted into bytecode, the `JIT` statically compiles portion of the bytecode:
- while compiling the code it performs some code optimization
- the JIT compiles code and optimizes it using run-time metrics

Java/Javascript are interpreted languages almost as efficient as compiled languages thanks to the `JIT`.

The Python implementation with CPython instead is much slower than compiled languages.

This because the CPython interpreter does not have the `JIT`.

The Python source code is converted to bytecode by the `CPython` and then converted to native code and executed by the `CPython`
virtual machine.

What makes it difficult for Python to have an efficient `JIT` is the fact that is dynamic typed.

For example it allows runtime patching of objects that require the object to be inspected (with a hashtable lookup or similar) whenever you invoke a method on the object.

Even if this is compiled, the compiler will have to generate code to do the method lookup at runtime.

Even if in general dynamic typed languages can be also statically typed, in practice there is not gain in efficiency,
because the compiler would generate the same code the interpreter would have to do to find its type, call the right function,... .

However, the `PyPy` project offers the `JIT` support to `Python` and it achieves higher performance than `CPython`.

### LLVM front-end concepts

The IR code that is generated is of type: `Value*`. 
The latter is the base class to represent all the values that are generated by the code as well as functions and instructions.

The peculiarity of LLVM is to use the SSA approach to generate IR code.
This means that each variable is assigned exactly once and declared before it is used: 
a new re-assignment is implemented by creating a new version of that variable.

The SSA facilitates the compiler to optimize code.
It is used in many tools such as: GCC, PyPy, Go Swift, PHP.

The `llvm` classes to generate IR code are:
- `LLVMContext`
- `IRBuilder`
- `Module`
- `BasicBlock`

### `LLVMContext`

It (opaquely) owns and manages the core "global" data of LLVM's core infrastructure, including the type and constant uniquing tables.

Since it says "opaquely" you're not supposed to know what it contains, what it does or what's used for.
Just think of it as a reference to the core LLVM "engine" that you should pass to the various methods that require a LLVMContext.

[Taken from this answer on StackOverflow](https://stackoverflow.com/questions/13184835/what-is-llvm-context)

### `IRBuilder`

The Builder object is a helper object that makes it easy to generate LLVM instructions.

Instances of the `IRBuilder` class template keep track of the current place to insert instructions and has methods to create new instructions.

The IR builder is supposed to make IR construction easier, that's all. It keeps track of an insertion point in the basic block, and in general using it should result in a bit shorter code.

The builder performs some optimization such as:
- constant folding (`return x+1+2` - > `return x+3`)

However, it is limited by the fact that it does all of its analysis inline with the code as it is built.

[Taken from this answer on StackOverflow](https://stackoverflow.com/questions/29206747/what-is-the-advantage-of-llvm-builder-over-manual-construction-of-ir-code)

### `Module`

A Module instance is used to store all the information related to an
LLVM module.
It is the main container class for the LLVM Intermediate Representation.

Modules are the top level container of all other LLVM Intermediate Representation (IR) objects. Each module directly contains a
list of globals variables, a list of functions, a list of libraries (or
other modules) this module depends on, a symbol table, and various data
about the target's characteristics.

It will own the memory for all of the IR that we generate.
 
A module maintains a GlobalValRefMap object that is used to hold all
constant references to global variables in the module.  When a global
variable is destroyed, it should have no entries in the GlobalValueRefMap.

### `BasicBlock`

A basic block is simply a container of instructions that execute sequentially. 
Basic blocks are Values because they are referenced by instructions such as branches and switch
tables.

The type of a BasicBlock is "Type::LabelTy" because the basic block represents a label to which a branch can jump.

A well formed basic block is formed of a list of non-terminating instructions followed by a single terminator instruction. Terminator
instructions may not occur in the middle of basic blocks, and must terminate
the blocks.

The BasicBlock class allows malformed basic blocks to occur because it may be useful in the intermediate stage of constructing or modifying a program.
However, the verifier will ensure that basic blocks are "well formed".


## About The Code

### Lexer

The `Lexer` is in charge of tokenizing the Kaleidoscope program represented as a raw string.
It iterates over the string tokens, keeping an internal cursor of the current position in the string.

It exposes the following API to iterate over the string:

- `void Next();`: move the cursor to the next token
- `std::string GetTokenValue();`: get the value of the current token as a string.
For example it can be a string representing a number or a function name or the function parameters
-  `Token GetTokenType();`: get the type of the token.
The valid tokens are represented by the `enum Token`: they can be identifier, number, def, operands, .... .

For some tokens `GetTokenValue` is useless since they have not a real value: for example the `+` token is of type `tok_plus` and its semantic value is the same of the type.

It is a lazy iterator meaning that it does not tokenize the whole text in one pass, but it sequentially finds the next token on demand.

The `Lexer` defines the available tokens: to provide the `Kaleidoscope` with a new feature such as the `for` keyword or the custom `!@` operator, the first thing to do is to add them to the `enum Token`.

### AST

The `ExprAST` is the abstract class to represent a node in the `Kaleidoscope` `AST`.

The method that defines the behaviour as node in the `AST` is:
- `llvm::Value *GenerateCode(CodeVisitor &);`: get the IR code representation of such node

It has also another method that is used only for testing reasons:
- `bool Equals(const std::unique_ptr<ExprAST> &);`: compare the nodes in the `AST`

The classes that implement `ExprAST` are the building blocks of `Kaleidoscope`. For example there is:
- `NumberExprAST`: represent a number
- `BinaryExprAST`: represent a binary operation. This class is representative of how the `Kaleidoscope` `AST` can be built recursively defining the actual tree structure. Specifically, the tree is implemented on the fly as a recursive visit.

Adding a new feature such as the `for` statement means adding a new class that implements the `ExprAST`.

### Parser

The `Parser` is in charge of creating the `AST` node for a given string.
Building the `AST` means giving the `Kaleidoscope` a semantic meaning. 

Specifically, the `Parser` exposes the functions:
- `std::unique_ptr<FunctionAST> HandleDefinition();`: to parse the function definition that starts with the `def` token
- `std::unique_ptr<PrototypeAST> HandleExtern();`: to parse the `extern` function
- `std::unique_ptr<FunctionAST> HandleTopLevelExpression();`: to parse the program as an anonymous function

Here it is important to note that whatever the string is, the result is a prototype or a function.
In fact when there is not `def` or `extern` token, the code is interpreted as an anonymous function.

Overall the parser defines the formal grammar of the language.
One of the most challenging operations it does is to implement the operator precedence.

### Interpreter

The `Interpreter` calls the `Parser` methods and the `AST` methods, to respectively obtain the `AST` nodes from the string and generate the IR code.

Even if each of the `AST` nodes returns the IR code, the latter is internally stored inside the `module_` instance variable of the `CodeVisitor` class.

The reason why each `AST` node returns the `IR` code, is to recursively add that code to the `module_` instance.


### Visitor

The `CodeVisitor` generates code for each of the different `AST` nodes.

This is where the `LLVM` libraries start to be used.

It has the following instance variables:

- `llvm::LLVMContext context_;`: it is the core LLVM engine and it is an opaque object (use it following the documentation but do not ask why).
- `llvm::IRBuilder<> builder_ = llvm::IRBuilder<>(context_);`: generate the IR code easily and in the correct place inside the `module_`.
- `std::unique_ptr<llvm::Module> module_;`: the container of the IR code with all the data structures such as the `symbol table` to disambiguate literals.
This is where the `IR` code for the prototype is added and when there is a function call it is retrieved.
- `std::map<std::string, llvm::Value *> named_values_`: custom map to easily get the function parameters value when generating the code for the function body.
It is used inside the function body to get the `IR` code for a the generic variable.

#### CodeVisitor

The constructor `CodeVisitor` initialize the `module_` that it will contain all the IR code.

In fact at the end of the parsing to get the generated code the `Interpreter`, will execute:
```c++
  std::string code;
  llvm::raw_string_ostream code_stream(code);
  code_stream << *visitor.module_;
  code_stream.flush();
```
to get the IR code from the module in which it is contained.

#### VisitNumber

The method `VisitNumber` returns the `IR` code for the `NumberExprAST`.

It is clear how internally `LLVM` keeps all the constant as unique and shared values.

In fact the `IR` code is obtained calling `llvm::ConstantFP::get(` instead of `llvm::ConstantFP::new(` or something like that

#### VisitVariable

The method `VisitVariable` returns the `IR` code for the `VariableExprAST`.

For now the only variables we have are the function parameters: if a variable is used in the code, it must be first declared as function parameter.

This is why is assumes to find and return the `IR` code for that variable inside the `named_values_` instance variable.

#### VisitBinary

Use the recursive calls to get the `IR` code and the `builder` to return the binary operation code.

#### VisitFunction

Add the prototype `IR` code to the `module_`, calling the `VisitPrototype` method.

If a function is defined more than once, only the last definition is considered.
All the previous definitions are deleted and the references are replaces with undefined type.

It creates a `BasicBlock` object that it will contain all the body instructions.

It populates the `named_values_` maps with the `IR` code for the function paramters.

Finally, the body code is generated, and inserted inside the `module_` using `builder_.CreateRet(ret_val)`;

## Code Style

- Filenames should be all lowercase with `_` as separator.
- The names of all types: classes, structs, type aliases, enums, and type template parameters,  should start with a capital letter and have a capital letter for each new word. No underscores. 
- The names of variables (including function parameters) and data members are all lowercase, with underscores between words. Data members of classes (but not structs) additionally have trailing underscores. For instance: `a_local_variable`, `a_struct_data_member`, `a_class_data_member_`.
- Variables declared constexpr or const, and whose value is fixed for the duration of the program, are named with a leading "k" followed by mixed case: `kDaysInAWeek` .
- Regular functions have mixed case; accessors and mutators may be named like variables.

## Acknowledgements

- https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html

- https://stackoverflow.com/questions/29206747/what-is-the-advantage-of-llvm-builder-over-manual-construction-of-ir-code

- https://stackoverflow.com/questions/13184835/what-is-llvm-context

- http://www.aosabook.org/en/llvm.html

- https://llvm.org/

- https://softwareengineering.stackexchange.com/questions/88645/are-dynamic-languages-always-interpreted

- https://en.wikibooks.org/wiki/Introduction_to_Programming_Languages/Precedence_and_Associativity
