#include <selenejs.h>

#include <iostream>

int functest(int a, std::string b)
{
	return a + 15;
}

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
	} catch (std::exception &e) {
		std::cout << "Error: " << e.what() << std::endl;
	}
}
