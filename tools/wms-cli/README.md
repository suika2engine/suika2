WMS (Watermelon Script)
=======================

WMS (Watermelon Script) is a plugin language for Suika2.
It is designed to be intuitive and easily written by anyone who has any experience with programming.

This directory provides a command line interface (CLI) for WMS.
New language features can be tested here.

## Syntax and Usage

### Defining and Calling Functions

A program starts from `main()` function.
```
func main() {
    // Print "Hello, world!"
    print("Hello, world!");
}
```

You can call other functions.
```
func main() {
    foo(0, 1 2);
}

func foo(a, b, c) {
    return a + b + c;
}
```

You can indirectly call functions using string variable.
```
func main() {
    myfunc = "foo";
    myfunc(0, 1 2);
}

func foo(a, b, c) {
    return a + b + c;
}

func myfunc(a, b, c) {
    // This function will not be called because foo() will be called in main().
}
```

### Types and Variables

You can use integer, floating point, string and array types.
```
func main() {
    // Integer
    a = 1;
    if(isint(a)) {
        print("a is int");
    }

    // Floating point
    b = 1.0;
    if(isfloat(b)) {
        print("b is float");
    }

    // String
    c = "string";
    if(isstr(c)) {
        print("c is string");
    }

    // Array (integer key)
    d[0] = 0;
    if(isarray(d)) {
        print("d is array");
    }

    // Array (string key)
    e["abc"] = 0;
    if(isarray(e)) {
        print("e is array");
    }

    // Array of array
    f["key"] = e;
}
```

### Loops

You can use a syntax for simple integer-range loops.
```
func main() {
    for(i in 0..9) {
        print(i);
    }
}
```

You can use a syntax for for-each loops.
```
func main() {
    a[0] = 0;
    a[1] = 1;
    a[2] = 2;

    for(v in a) {
        print(v);
    }
}
```

You can use key-value pair in for-each loops.
```
func main() {
    a["key1"] = 0;
    a["key2"] = 1;
    a["key3"] = 2;

    for(k, v in a) {
        print(k + "=" + v);
    }
}
```

You can use normal `while` syntax.
```
func main() {
    a = 10;
    while (a > 0) {
        print(a);
        a = a - 1;
    }
}
```

Note that you can use `break` and `continue` in `for` and `while` loops.
```
func main() {
    for(i in 0..9) {
        if(i == 2) {
            continue;
        }
        if(i == 7) {
            break;
        }
        print(i);
    }
```

### Branches

You can make branches by `if` - `else if` - `else` syntax.
```
func main() {
    a = foo();
    if(a > 10) {
        print("a > 10");
    } else if(a > 5) {
        print("a > 5");
    } else {
        print("else");
    }
}

func foo() {
    return 6;
}
```

### Arrays

An array element has a key, and the key must be one of integer, floating point and string.
Keys of integer, floating point and string can be mixed.

To create an array, assign a value with `[]` syntax.
```
func main() {
    a[0] = 0;
    print(a);
}
```

To remove an array element, use `remove()`.
```
func main() {
    a[0] = 0;
    remove(a, 0);
    print(a);
}
```

To get the size of an array, use `size()`.
```
func main() {
    a[0] = 0;
    print(size(a));
}
```

### Conversions

Integer to string.
```
   s = "" + 123;
```

Floating point to string.
```
   s = "" + 1.23;
```

Integer to floating point.
```
   f = 0.0 + 123;
```

String to integer.
```
   i = 0 + "123";
```

String to floating point.
```
   f = 0.0 + "1.23";
```

### Reading String

In the CLI, you can use `readline()` function to input string from the terminal.
It is for debug usage only.
```
func main() {
    a = readline();
    print(a);
}
```

Note that file I/O is not implemented in the CLI by default
but you can implement it by FFI.

### Calling C Functions

WMS has Foreign Function Interface (FFI).
You can register C functions to a WMS runtime as FFI functions,
and you can call them inside the WMS code.

The CLI includes `hello()` FFI function to show example.
```
func main() {
    array["hello"] = "hello";
    a = hello(1, 3.14, "str", array);
    print(a["hello"]);
}
```

A FFI function can accept up to 16 arguments and can return a value.
There are so many APIs to manipulate int, float, string and array values.
