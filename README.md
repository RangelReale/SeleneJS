# SeleneJS

Simple C++11 friendly header-only bindings to the [Duktape](http://duktape.org/) javascript engine.
This is a direct source port of [Selene](https://github.com/jeremyong/Selene).

## Requirements

- Cmake 2.8+
- Duktape 1.5+
- C++11 compliant compiler

## Usage

SeleneJS is a headers-only library so you just need to include
"selenejs.h" to use this project.

To build the tests, do the following:

```
mkdir build
cd build
cmake ..
make
```

This will build a `test_runner` executable that you can run.

## Usage

### Establishing Duktape Context

```c++
using namespace seljs;
State state; // creates a Duktape context
```

When a `seljs::State` object goes out of scope, the Duktape context is
automatically destroyed in addition to all objects associated with it
(including C++ objects).

### Accessing elements

```javascript
// test.js
foo = 4;
bar = {};
bar[3] = "hi";
bar["key"] = "there";
```

```c++
seljs::State state;
state.Load("/path/to/test.js");
assert(state["foo"] == 4);
assert(state["bar"][3] == "hi");
assert(state["bar"]["key"] == "there");
```

When the `[]` operator is invoked on a `seljs::State` object, a
`seljs::Selector` object is returned. The `Selector` is type castable to
all the basic types that Duktape can return.

If you access the same element frequently, it is recommended that you
cache the selector for fast access later like so:

```c++
auto bar3 = state["bar"][3]; // bar3 has type seljs::Selector
bar3 = 4;
bar3 = 6;
std::cout << int(bar3) << std::endl;
```

### Calling JavaScript functions from C++

```javascript
// test.js

function foo() {
}

function add(a, b) {
  return a + b;
}

mytable = {}
function mytable.foo() {
    return 4
}
```

```c++
seljs::State state;
state.Load("/path/to/test.js");

// Call function with no arguments or returns
state["foo"]();

// Call function with two arguments that returns an int
// The type parameter can be one of int, double, std::string,
// bool, or unsigned int
int result = state["add"](5, 2);
assert(result == 7);


// Call function in table
result = state["mytable"]["foo"]();
assert(result == 4);
```

### Calling Free-standing C++ functions from javascript

```c++
int my_multiply(int a, int b) {
    return (a*b);
}

seljs::State state;

// Register the function to the javascript global "c_multiply"
state["c_multiply"] = &my_multiply;

// Now we can call it (we can also call it from within javascript)
int result = state["c_multiply"](5, 2);
assert(result == 10);
```

You can also register functor objects, lambdas, and any fully
qualified `std::function`. See `test/interop_tests.h` for details.

#### Accepting JavaScript functions as Arguments

To retrieve a javascript function as a callable object in C++, you can use
the `seljs::function` type like so:

```javascript
// test.js

function add(a, b) {
    return a+b;
}

function pass_add(x, y) {
    take_fun_arg(add, x, y);
}
```

```c++
int take_fun_arg(seljs::function<int(int, int)> fun, int a, int b) {
    return fun(a, b);
}

seljs::State state;
state["take_fun_arg"] = &take_fun_arg;
state.Load("test.js");
assert(state["pass_add"](3, 5) == 8)
```

The `seljs::function` type is pretty much identical to the
`std::function` type excepts it holds a `shared_ptr` to a JavaScript
reference. Once all instances of a particular `seljs::function` go out
of scope, the JavaScript reference will automatically become unbound. Simply
copying and retaining an instance of a `seljs::function` will allow it
to be callable later. You can also return a `seljs::function` which will
then be callable in C++ or JavaScript.

### Running arbitrary code

```c++
seljs::State state;
state("x = 5");
```

After running this snippet, `x` will have value 5 in the Duktape runtime.
Snippets run in this way cannot return anything to the caller at this time.

### Registering Classes

```c++
struct Bar {
    int x;
    Bar(int x_) : x(x_) {}
    int AddThis(int y) { return x + y; }
};

seljs::State state;
state["Bar"].SetClass<Bar, int>("add_this", &Bar::AddThis);
```

```javascript
bar = new Bar.Bar(5);
// bar now refers to a new instance of Bar with its member x set to 5

x = bar.add_this(2);
// x is now 7

bar = null;
/*[[ the bar object will be destroyed the next time a garbage
     collection is run ]]*/
```

The signature of the `SetClass` template method looks like the
following:

```c++
template <typename T, typename... CtorArgs, typename... Funs>
void Selector::SetClass(Funs... funs);
```

The template parameters supplied explicitly are first `T`, the class
you wish to register followed by `CtorArgs...`, the types that are
accepted by the class's constructor. In addition to primitive types,
you may also pass pointers or references to other types that have been
or will be registered. Note that constructor overloading
is not supported at this time. The arguments to the `SetClass`
function are a list of member functions you wish to register (callable
from JavaScript). The format is [function name, function pointer, ...].

After a class is registered, C++ functions and methods can return
pointers or references to JavaScript, and the class metatable will be
assigned correctly.

#### Registering Class Member Variables

For convenience, if you pass a pointer to a member instead of a member
function, Selene will automatically generate a setter and getter for
the member. The getter name is just the name of the member variable
you supply and the setter has "set_" prepended to that name.

```c++
// Define Bar as above
seljs::State state;
state["Bar"].SetClass<Bar, int>("x", &Bar::x);
```

```javascript
// now we can do the following:
bar = new Bar.Bar(4);

print(bar.x()); // will print '4'

bar.set_x(-4);
print(bar.x()); // will print '-4'
```

Member variables registered in this way which are declared `const`
will not have a setter generated for them.

### Registering Object Instances

You can also register an explicit object which was instantiated from
C++. However, this object cannot be inherited from and you are in
charge of the object's lifetime.

```c++
struct Foo {
    int x;
    Foo(int x_) : x(x_) {}

    int DoubleAdd(int y) { return 2 * (x + y); }
    void SetX(int x_) { x = x_; }
};

seljs::State state;

// Instantiate a foo object with x initially set to 2
Foo foo(2);

// Binds the C++ instance foo to an object also called foo in JavaScript along
// with the DoubleAdd method and variable x. Binding a member variable
// will create a getter and setter as illustrated below.
// The user is not required to bind all members
state["foo"].SetObj(foo,
                    "double_add", &Foo::DoubleAdd,
                    "x", &Foo::x);

assert(state["foo"]["x"]() == 2);

state["foo"]["set_x"](4);
assert(foo.x == 4);

int result = state["foo"]["double_add"](3);
assert(result == 14);
```

In the above example, the functions `foo.double_add` and `foo.set_x`
will also be accessible from within JavaScript after registration occurs. As
with class member variables, object instance variables which are
`const` will not have a setter generated for them.

## Writeups

You can read more about the Lua project in the three blogposts that describes it:

- [first part](http://www.jeremyong.com/blog/2014/01/10/interfacing-lua-with-templates-in-c-plus-plus-11/).
- [second part](http://www.jeremyong.com/blog/2014/01/14/interfacing-lua-with-templates-in-c-plus-plus-11-continued)
- [third part](http://www.jeremyong.com/blog/2014/01/21/interfacing-lua-with-templates-in-c-plus-plus-11-conclusion)
- [ref-qualifier usage](http://www.jeremyong.com/blog/2014/02/15/using-ref-qualifiers/)

There have been syntax changes in library usage but the underlying
concepts of variadic template use and generics is the same.
