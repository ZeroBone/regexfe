# regexfe

A simple regular expression matcher for text files.

## Installation

Download the jar file of the form `grammax-x.x.x.jar` from the [Grammax releases page](https://github.com/ZeroBone/Grammax/releases).

Clone this repository, go to the `src` directory and place the `grammax-x.x.x.jar` file there.

Located in the same `src` directory, run
```bash
java -jar grammax-x.x.x.jar grammar.grx
```
Obviously, Java has to be installed for this to work.

This generates the code for the parser (`src/Parser.h` file). Now we need to build the dependencies and compile the project.

To build the dependencies, run
```bash
./build_dependencies.sh
```

Now we can compile the project in the standard way using `cmake`. If compiling for the first time, create a directory `build` and run
```bash
cmake -S . -B ./build/
cmake --build ./build
```
For recompilation only the last command suffices.

## Usage

The name of the produced binary is `regexfe`. It can be used to match every line of a given text file against a regular expression.
For example,
```bash
./build/regexfe "[^a-z].*" LICENSE
```
returns true for all lines that start with a non-capital English letter, and false otherwise.