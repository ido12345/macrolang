# macrolang
A text preprocessor designed to work as a macro system for every programming language

# Philosophy
The idea is that you shouldn't need a built in macro system for every language that is different, complicated, and hard to remember.
There should only be one macro system that is simple, powerful, and can be used anywhere.

# Inspiration
When working with macros in C, I often hit a limitation that prevented me from doing what I wanted to do, for example limitations with variadics.
So I decided I should work on my own macro system, and then it hit me, a macro system does not have to be bound to the rules of the language.
A macro system simply needs to process text, nothing more.

Simple Example:
```c
----- INPUT -----
#macro FIVE 3
#macro THREE FIVE

THREE FIVE

#macro x HiFromX
#macro A(x,y) x->y

A(x,4)

#macro StringMe abcdefg
#macro STR(x) "x"

STR(String Me)
STR(StringMe)

#macro Test(A) HiFromTest

#macro arg A(x,HiFromY)

STR(arg)

STR(A(x,HiFromY))

example of empty arguments:
STR(A(STR(A(,)),))

A(A,4)
----- INPUT -----

----- MACROS -----
[( KEY: [(Text): "FIVE"], VALUE: [(Number): "3"]), ( KEY: [(Text): "THREE"], VALUE: [(Text): "FIVE"]), ( KEY: [(Text): "x"], VALUE: [(Text): "HiFromX"]), ( KEY: [(Text): "A", (Text): "x", (Text): "y"], VALUE: [(Text): "x", (Symbol): "-", (Symbol): ">", (Text): "y"]), ( KEY: [(Text): "StringMe"], VALUE: [(Text): "abcdefg"]), ( KEY: [(Text): "STR", (Text): "x"], VALUE: [(Symbol): """, (Text): "x", (Symbol): """]), ( KEY: [(Text): "Test", (Text): "A"], VALUE: [(Text): "HiFromTest"]), ( KEY: [(Text): "arg"], VALUE: [(Text): "A", (Symbol): "(", (Text): "x", (Symbol): ",", (Text): "HiFromY", (Symbol): ")"])]
----- MACROS -----

----- OUTPUT -----

3 3


HiFromX->4


"String Me"
"abcdefg"



"HiFromX->HiFromY"

"HiFromX->HiFromY"

example of empty arguments:
""->"->"

A->4
----- OUTPUT -----
```
