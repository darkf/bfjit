#include <cstdio>
#include <vector>
#include <stack>
#include <fstream>
#include <asmjit/asmjit.h>
using namespace asmjit;
using namespace asmjit::x86;

int main(int argc, char *argv[]) {
	std::ifstream fs;
	std::vector<char> cells(1024); // construct 1024 cells
	std::stack<std::pair<Label, Label>> block_labels;
	JitRuntime runtime;
	X86Compiler c(&runtime);
	cells.assign(1024, 0);

	if(argc < 2) {
		printf("USAGE: %s FILE", argv[0]);
		return 1;
	}

	fs.open(argv[1]);

	c.addFunc(kFuncConvHost, FuncBuilder1<void, char*>());

	X86GpVar cells_ptr(c, kVarTypeIntPtr, "cells_ptr");
	c.setArg(0, cells_ptr);
	X86GpVar a(c, kVarTypeInt8, "a");
	X86GpVar b(c, kVarTypeInt32, "b");

	X86GpVar _putchar(c, kVarTypeIntPtr, "_putchar");
	c.mov(_putchar, intptr_t(&putchar));

	X86GpVar _getchar(c, kVarTypeIntPtr, "_getchar");
	c.mov(_getchar, intptr_t(&getchar));

	char ch;
	while(fs >> ch) {
		switch(ch) {
			case '+':
				c.inc(byte_ptr(cells_ptr));
				break;
			case '-':
				c.dec(byte_ptr(cells_ptr));
				break;
			case '<':
				c.dec(cells_ptr);
				break;
			case '>':
				c.inc(cells_ptr);
				break;
			case '.': {
				c.mov(b, byte_ptr(cells_ptr));
				X86CallNode *call = c.call(_putchar, kX86FuncConvCDecl, FuncBuilder1<int, int>());
				call->setArg(0, b);
				break;
			}
			case ',': {
				X86CallNode *call = c.call(_getchar, kX86FuncConvCDecl, FuncBuilder0<int>());
				call->setRet(0, b);
				c.mov(byte_ptr(cells_ptr), b);
				break;
			}
			case '[': {
				// a pair of addresses for (start of loop, end of loop)
				block_labels.emplace(std::make_pair(Label(c), Label(c)));
				auto& label_pair = block_labels.top();

				c.bind(label_pair.first);
				c.mov(a, byte_ptr(cells_ptr));
				c.test(a, a);
				c.jz(label_pair.second);
				break;
			}
			case ']': {
				auto& label_pair = block_labels.top();
				block_labels.pop();
				c.jmp(label_pair.first);
				c.bind(label_pair.second); // back-patch the label to refer to here (after the ])
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