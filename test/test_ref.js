function add(a, b) {
   return a+b
}

function subtract(a, b) {
   return a-b
}

function pass_add(x, y) {
   return take_fun_arg(add, x, y)
}

function pass_sub(x, y) {
   return take_fun_arg(subtract, x, y)
}

a = 4

function mutate_a(new_a) {
   a = new_a
}

test = ""

function foo() {
   test = "foo"
}

function bar() {
   test = "bar"
}
