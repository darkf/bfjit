#include <cstdio>
#include <asmjit/asmjit.h>
#include <vector>
#include <stack>
using namespace asmjit;
using namespace asmjit::x86;

int main(int argc, char *argv[]) {
	std::vector<char> cells(1024); // construct 256 cells
	std::stack<std::pair<Label, Label>> block_labels;
	JitRuntime runtime;
	X86Compiler c(&runtime);

	cells.assign(1024, 0);

	const char code[] = "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.";

	c.addFunc(kFuncConvHost, FuncBuilder1<void, char*>());

	X86GpVar cells_ptr(c, kVarTypeIntPtr, "cells_ptr");
	c.setArg(0, cells_ptr);
	X86GpVar a(c, kVarTypeInt8, "a");
	X86GpVar b(c, kVarTypeInt32, "b");

	X86GpVar _putchar(c, kVarTypeIntPtr, "_putchar");
	c.mov(_putchar, intptr_t(&putchar));

	//Label lbl(c);
	//Label lbl2(c);

	for(char ch : code) {
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