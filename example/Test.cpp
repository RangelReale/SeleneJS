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

struct Bar {
	int x;
	Bar(int num) { x = num; }

	std::string Print(int y) {
		return std::to_string(x) + "+" + std::to_string(y);
	}

	void SetX(int x2) {
		x = x2;
	}

	int GetX() {
		return x;
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

		state["Bar"].SetClass<Bar, int>("print", &Bar::Print, "get_x", &Bar::GetX);
		state("var abar = new Bar.Bar(8);");
		auto ab = state["abar"];
		std::cout << ab.type() << std::endl;
		auto abx = ab["get_x"];
		std::cout << abx.type() << std::endl;
		//int gx = state["abar"]["get_x"]();
		state("var barx = abar.get_x();");
		state("var barp = abar.print(2);");

		int result1 = state["barx"];
		std::string result2 = state["barp"];

		std::cout << "Bar: barx=" << result1 << " barp=" << result2 << std::endl;

	} catch (std::exception &e) {
		std::cout << "Error: " << e.what() << std::endl;
	}
}
