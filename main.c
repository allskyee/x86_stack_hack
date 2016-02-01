#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <dlfcn.h>

struct s_func_ptrs {
	const char* name;
	int n_params;
	void* (*func)(void);
} func_ptrs[] = 
	{ {"add_u8_2", 2, NULL}, 
	  {NULL, 0, NULL} 
	};

struct s_stack_frame {
	long save_ebp;
	void* ret_addr;
	const char* param_name;
	long params[0];
};

void* __attribute__((stdcall)) func_route(const char* name, ...) 
{
	struct s_stack_frame* sf = __builtin_frame_address(0);
	struct s_func_ptrs* fp = func_ptrs;
	int i;

	for(; fp->name && strcmp(fp->name, name) != 0; fp++);
	assert(fp);

#if 0
	printf("[%s]\n", sf->param_name);
	printf("%lx\n", sf->params[0]);
	printf("%lx\n", sf->params[1]);
#endif
	
	// artificially call func
	for (i = fp->n_params - 1; i >= 0; i--) {
		asm volatile ("movl %0, %%eax; push %%eax;" : : "r"(sf->params[i]) : "%eax");
	}
	return fp->func();
}

void test()
{
	unsigned char i, j, k;
	unsigned char r = (unsigned char)func_route("add_u8_2", (unsigned char)1, (unsigned char)2);

	printf("%d\n", r);
}

int main(int argc, char* argv[])
{
	void* handle;
	struct s_func_ptrs* fp = func_ptrs;
	int i;

	// open dynamic lib
	handle = dlopen("./lib/libtest.so", RTLD_LAZY | RTLD_GLOBAL); 
	if (!handle) { 
		fprintf(stderr, "unable to open the library\n");
		return -1;
	}

	// set func pointers read from dynamic lib
	while (fp->name) {
		fp->func = dlsym(handle, fp->name);
		if (!fp->func) { 
			fprintf(stderr, "unable to load symbol\n");
			return -1;
		}
		printf("loaded func [%s]\n", fp->name);
		fp++;
	}

	// test func calls
	test();

	return 0;
}
