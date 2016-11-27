function foo() {
}

function add(a, b) {
   return a + b
}

function execute() {
   return cadd(5, 6);
}

mytable = {
  foo: function() {
    return 4
  }
};

function embedded_nulls() {
   return "\0h\0i"
}

my_global = 4

my_table = {}
my_table[3] = "hi"
my_table["key"] = 6.4

nested_table = {}
nested_table[2] = -3;
nested_table["foo"] = "bar";

my_table["nested"] = nested_table

global1 = 5
global2 = 5

function set_global() {
   global1 = 8
}

should_be_one = 0

function should_run_once() {
   should_be_one = should_be_one + 1
}