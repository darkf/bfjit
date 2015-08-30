#include <cstdio>
#include <asmjit/asmjit.h>
#include <vector>
using namespace asmjit;
using namespace asmjit::x86;

int main(int argc, char *argv[]) {
	std::vector<char> cells(256); // construct 256 cells
	JitRuntime runtime;
	X86Compiler c(&runtime);

	cells.assign(256, 0);

	const char code[] = "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.>+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++.";

	c.addFunc(kFuncConvHost, FuncBuilder1<void, char*>());

	X86GpVar cells_ptr(c, kVarTypeIntPtr, "cells_ptr");
	c.setArg(0, cells_ptr);
	X86GpVar a(c, kVarTypeInt32, "a");
	//X86GpVar b(c, kVarTypeInt32, "b");

	X86GpVar _putchar(c, kVarTypeIntPtr, "_putchar");
	c.mov(_putchar, intptr_t(&putchar));

	for(char ch : code) {
		switch(ch) {
			case '+':
				c.inc(byte_ptr(cells_ptr));
				break;
			case '<':
				c.sub(cells_ptr, 4);
				break;
			case '>':
				c.add(cells_ptr, 4);
				break;
			case '.': {
				c.mov(a, byte_ptr(cells_ptr));
				X86CallNode *call = c.call(_putchar, kX86FuncConvCDecl, FuncBuilder1<int, int>());
				call->setArg(0, a);
				break;
			}
			case '\0':
				break;
			default:
				printf("error: unknown char '%c'\n", ch);
				return 1;
		}
	}

	c.endFunc();

	void* funcPtr = c.make();
	typedef int (*FuncType)(char *ptr);

	// run JITed code
	FuncType func = asmjit_cast<FuncType>(funcPtr);
	func(cells.data());

	runtime.release((void*)func);

	return 0;
}