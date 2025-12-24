# Lowkey (lkey) - A Readable Programming Language

# Lowkey (lkey)

> *"Programming should be a conversation, not a puzzle."*

Lowkey is a simple, intuitive programming language that reads like plain English while maintaining the power of traditional programming. Perfect for beginners, educational purposes, and rapid prototyping.

## 🚀 Quick Start

```bash
# Clone and build
git clone https://github.com/yourusername/lkey.git
cd lkey
mkdir build && cd build
cmake .. && make

# Run your first program
./lkeycpp hello.lkey
```

## 📦 Table of Contents

- [🚀 **Getting Started**](#-installation--build)
- [📖 **Language Reference**](#-language-reference)
- [📘 **Examples**](#-examples)
- [🏗 **Project Structure**](#-project-structure)
- [📝 **Advanced Features**](#-advanced-features)

---

## 🚀 Installation & Build

### Prerequisites
- A C++ compiler (GCC, Clang, or MSVC)
- CMake (version 3.10 or higher)
- Git (optional, for source control)

### Building from Source

```bash
# Clone the repository
git clone https://github.com/LokajithPT/lkey.git
cd lkey

# Create build directory
mkdir build && cd build

# Configure and build
cmake .. && make

# Run tests
ctest
```

### Build Options

The CMake build system supports several configurations:

```bash
# Debug build with all warnings
cmake -DCMAKE_BUILD_TYPE=Debug .. && make

# Release build with optimizations
cmake -DCMAKE_BUILD_TYPE=Release .. && make
```

---

## 📖 Language Reference

### Philosophy
Lowkey follows these principles:
- **Readability First**: Code should read like a conversation
- **Explicit is Better**: No cryptic symbols, clear intent
- **Consistency**: Uniform patterns throughout the language
- **Minimalism**: Simple syntax for complex ideas

### 1. Comments

Comments start with `comment` and continue to the end of the line:

```lowkey
comment This is a single line comment
comment This is also a comment that spans
  multiple lines with proper indentation.
```

### 2. Variables & Data Types

#### Variable Declaration
```lowkey
var name is "Alice"
var age is 25
var is_active is true
var score is 95.5
```

#### Type System
Lowkey uses dynamic typing with these types:
- **String**: `"hello world"`
- **Number**: `42`, `3.14`
- **Boolean**: `true`/`false` (represented as `1`/`0`)

#### Variable Reassignment
```lowkey
name is "Bob"
age is age plus 1
score is score minus 5
```

### 3. Output

#### Printing to Console
```lowkey
say "Hello, World!"
say "Your age is " plus age
say "Result: " plus (10 plus 5)
```

#### String Operations
```lowkey
say "Hello, " plus name plus "!"
var greeting is "Hi there, " plus name
```

### 4. Mathematical Operations

Lowkey uses word-based operators for better readability:

| Operation | Keyword | Example | Result |
|-----------|---------|---------|--------|
| Addition | `plus` | `a plus b` | a + b |
| Subtraction | `minus` | `a minus b` | a - b |
| Multiplication | `into` | `a into b` | a * b |
| Division | `div` | `a div b` | a / b |
| Modulus | `reminder` | `a reminder b` | a % b |

#### Operator Precedence
1. Parentheses `()`
2. `into` and `div`
3. `plus` and `minus`
4. Comparison operators

#### Examples
```lowkey
var result is (10 plus 5) into 2
var area is 3.14 into radius into radius
say "Final answer: " plus result
```

### 5. Comparisons & Logic

#### Comparison Operators
| Operator | Keyword | Example |
|-----------|---------|---------|
| Equal to | `equal` | `a equal b` | a == b |
| Not equal to | `not equal` | `not a equal b` | a != b |
| Greater than | `greater than` | `a greater than b` | a > b |
| Less than | `lesser than` | `a lesser than b` | a < b |

#### Boolean Logic
| Operator | Keyword | Example |
|-----------|---------|---------|
| AND | `and` | `a and b` | a && b |
| OR | `or` | `a or b` | a \|\| b |
| NOT | `not` | `not a` | !a |

#### Examples
```lowkey
when (age greater than 18) then
    say "You are an adult"
when nothing then
    say "You are a minor"

when (is_student and has_permission) then
    say "Access granted"
when (not has_permission or not is_student) then
    say "Access denied"
```

### 6. Control Flow

#### Conditional Statements (If-Then-Else)

```lowkey
when (score greater than 90) then
    say "Excellent!",
when (score greater than 80) then
    say "Good!",
when nothing then
    say "Keep trying!"
```

#### Switch-like Logic
You can chain multiple conditions:

```lowkey
when (grade equal "A") then
    say "Perfect score!",
when (grade equal "B") then
    say "Great job!",
when nothing then
    say "Keep studying!"
```

### 7. Loops

#### Range Loops
Iterate from a start value to an end value (inclusive):

```lowkey
with i from 1 to 10 then
    say "Count: " plus i
```

#### While Loops
Continue while a condition is true:

```lowkey
with (x greater than 0) then
    say x is x minus 1
    x is x minus 1
```

#### For-Each Loops
Iterate over items in a string or collection:

```lowkey
with sentence is word as words then
    say "Word: " plus word

with sentence is char as letters then
    say "Letter: " plus letter
```

### 8. Arrays (Collections)

#### Array Creation
```lowkey
var numbers is (1, 2, 3, 4, 5)
var mixed is (1, "hello", true, 3.14)
var nested is (1, (2, 3), (4, 5))
```

#### Array Access
```lowkey
var first is numbers(0)    # First element
var third is numbers(2)    # Third element
var nested_second is nested(1)(1)  # Second element of nested array
```

#### Array Operations
```lowkey
var doubled is numbers into numbers
say "Sum: " plus (doubled(0) plus doubled(4))

# Creating a new array by combining
var combined = numbers plus (6, 7, 8)
```

### 9. Functions

#### Function Definition
```lowkey
how to add with a, b so
    var result is a plus b
    please give result
thats how

how to calculate_area with radius so
    var pi is 3.14
    var area is pi into radius into radius
    please give area
thats how
```

#### Function Calls
```lowkey
how to greet with name so
    say "Hello, " plus name plus "!"
thats how

var greeting is add("World", "!")
say greeting
```

#### Return Values
```lowkey
how to check_age with age so
    when (age greater than 18) then
        please give true
    when nothing then
        please give false
thats how

var is_adult is check_age(25)
```

#### Recursion
```lowkey
how to factorial with n so
    when (n lesser than 2) then
        please give 1
    when nothing then
        please give n into factorial(n minus 1)
thats how

var result is factorial(5)
```

### 10. Classes (Object-Oriented)

#### Class Declaration
```lowkey
hows to Person has
    var name is ""
    var age is 0
    how to init with name, age so
        name is name
        age is age
    thats how

hows to Animal has
    how to speak with sound so
        please give "Generic animal sound"
    thats how

hows to Calculator has
    how to add with a, b so
        please give a plus b
    how to multiply with a, b so
        please give a into b
    thats how
thats how
```

#### Object Creation & Method Calls

```lowkey
var person is Person
person.init("Alice", 30)
person.speak()

var calc is Calculator
var result is calc.add(10, 5)
calc.multiply(result, 2)
```

#### Questions (Natural Language Queries)

Classes can answer natural language questions:

```lowkey
questions name are "What is your name?"
ans we end it with "success"

questions age is 25
questions is_active is true
questions status is "Ready"
ans we end it with "All systems operational"

questions description is "Tell me about this calculator"
ans we end it with "Basic arithmetic calculator with add, multiply methods"
```

### 11. Error Handling

Lowkey provides clear error messages with line numbers:

```lowkey
# Example of syntax error
say x  # Missing variable declaration

# Example of runtime error
# Will show line number and helpful message
```

---

## 📘 Examples

### Hello World
```lowkey
say "Hello, World!"
```

### Simple Calculator
```lowkey
how to calculate_sum with a, b so
    var result is a plus b
    please give result
thats how

how to calculate_diff with a, b so
    var result is a minus b
    please give result
thats how

var sum is calculate_sum(10, 5)
var diff is calculate_diff(10, 3)
say "Sum: " plus sum
say "Diff: " plus diff
```

### Guess the Number Game
```lowkey
var target is 42
var guess is 0

with (guess lesser than target) then
    say "Too low!",
when (guess greater than target) then
    say "Too high!",
when nothing then
    say "Just right!"

guess is read
```

### Text Processing
```lowkey
var text is "hello world"
var word_count is 0

with text is char as char as letters then
    word_count is word_count plus 1
say word_count
```

### Basic CRUD Operations
```lowkey
# Create, Read, Update, Delete operations
var users is ()

hows to create_user with username, email so
    var new_user is (username, email)
    users is users into new_user
    say "User created: " plus username

hows to find_user with username so
    with users as user where user.name is username then
        please give user
    when nothing then
        say "User not found"
```

---

## 🏗 Project Structure

```
lkey/
├── src/                    # Source files
│   ├── main.cpp           # Entry point
│   ├── Lexer.cpp          # Tokenization
│   ├── Parser.cpp          # AST construction
│   └── Interpreter.cpp     # Execution engine
├── include/                # Header files
│   ├── Token.h
│   ├── Lexer.h
│   ├── Parser.h
│   ├── Expr.h
│   ├── Stmt.h
│   └── Interpreter.h
├── tests/                  # Test files
│   ├── *.lkey           # Language test cases
│   └── test_runner.cpp  # Test framework
├── build/                 # Build artifacts
├── CMakeLists.txt          # Build configuration
└── README.md              # This file
```

---

## 📝 Advanced Features

### String Manipulation
```lowkey
var text is "Hello, World!"
var reversed is ""

# Manual string reversal
with text as char as letters then
    reversed is text plus reversed

var length is 0
with text as char as letters then
    length is length plus 1
```

### Regular Expressions (Planned)
Future versions will include pattern matching:

```lowkey
when text matches "hello" then
    say "Greeting detected"

when text matches "[0-9]+" then
    say "Phone number detected"
```

### File I/O (Planned)
Future versions will support file operations:

```lowkey
var content is read_file("data.txt")
write_file("output.txt", content)
```

### JSON Support (Planned)
Future versions will include JSON handling:

```lowkey
var data is parse_json("{\"name\": \"Alice\", \"age\": 25}")
say "Name: " plus data.name
say "Age: " plus data.age
```

---

## 🎯 Why Lowkey?

### For Beginners
- **No cryptic symbols**: No need to memorize `{}[];` vs `{}` 
- **Self-documenting**: Code reads like English comments
- **Lower learning curve**: Focus on logic, not syntax
- **Debuggable**: Errors are in plain English

### For Education
- **Teaches programming concepts**: Variables, loops, functions, OOP
- **Immediate feedback**: See results instantly
- **Progressive complexity**: Start simple, add features gradually

### For Prototyping
- **Quick to write**: No boilerplate or setup required
- **Rapid iteration**: Test ideas quickly
- **Clear syntax**: Easy to modify and extend
- **Domain specific**: Can be tailored for specific use cases

### For Code Reviews
- **Readable by default**: Anyone can understand the code
- **Explicit logic**: Business rules are clear and unambiguous
- **Natural documentation**: Code comments in plain English
- **Maintainable**: Simple syntax is easy to modify

---

## 🛠 Contributing

We welcome contributions! See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

### Development Guidelines
- Keep it simple and readable
- Test your changes with existing test suite
- Follow the existing code style
- Update documentation for new features
- Ensure backward compatibility when possible

---

## 📚 License

MIT License - Feel free to use this in your projects!

---

*"Programming should be a conversation, not a puzzle."*
