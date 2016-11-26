#include <selenejs.h>

#include <iostream>

int functest(int a, std::string b)
{
	return a + 15;
}

struct Foo {
	int x;
	const int y;
	Foo(int x_) : x(x_), y(3) {}
	int GetX() { return x; }
	int DoubleAdd(int y) {
		return 2 * (x + y);
	}
	void SetX(int x_) {
		x = x_;
	}
};

int main() {
	try
	{
		seljs::State state;

		state(R"JS(
	var Info = {
		version: 1,
		description: 'Test JS Plugin',
		test1: 41,
		test2: true,

		functest: function(v1, v2)
		{
			print(this);
			return "Info.functest = " + v1 + "-" + v2+" [" + this.description + "]";	
		}
	};
	)JS");

		std::string description = state["Info"]["description"];
		std::cout << description << std::endl;

		state["functest"] = &functest;
		state("print(functest(5, 'aaa'));");

		Foo foo_instance(1);
		state["foo_instance"].SetObj(foo_instance, "double_add", &Foo::DoubleAdd);
		const int answer = state["foo_instance"]["double_add"](3);
		std::cout << "Answer: " << answer << std::endl;
	} catch (std::exception &e) {
		std::cout << "Error: " << e.what() << std::endl;
	}
}
