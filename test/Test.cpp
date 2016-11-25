#include <selenejs.h>

int main() {
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
}
