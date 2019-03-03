#include "context.hpp"
#include <iostream>
#include <cstring>

#include <dlfcn.h>

Context::Context(){
	push_frame(0);
}

Context::~Context(){
}

void Context::push_frame(int arg_count){
	/*
	 * Because of the hacky way I store local variables 
	 * and call frames we need to move arguments from the
	 * previous call frame into this one. Args are evaluated
	 * in the previous call frame so variable lookups work.
	 */
	//puts("pushing frame");
	std::vector<Object *> args;
	for(int i = 0; i < arg_count; i++){
		args.push_back(pop());
	}
	stack.push_front(std::vector<Object*>());

	for(int i = args.size()-1; i >= 0; i--){
		push(args[i]);
	}
}

void Context::pop_frame(void){
	if(stack.size() <= 1) return;
	//puts("popping frame");
	stack.pop_front();
}

void Context::push(Object *o){
	std::vector<Object *> &callframe = this->stack.front();
	callframe.push_back(o);
}

Object *Context::pop(void){
	std::vector<Object *> &callframe = stack.front();
	if(callframe.empty()){
		return new Object(UNIT);
	}
	Object *o = callframe[callframe.size()-1];
	callframe.pop_back();
	return o;
}

void Context::link(int lnk){
	ret_stack.push_back(lnk);
}

int Context::ret(Object *o){
	int out = -1;
	if(!ret_stack.empty()){
		out = ret_stack[ret_stack.size()-1];
		ret_stack.pop_back();
	}
	pop_frame();
	push(o);
	return out;
}

Object *Context::get(int i){
	std::vector<Object *> &callframe = this->stack.front();
	if(i >= 0 && i < callframe.size()){
		return callframe[i];
	}
	return new Object(UNIT);
}

void Context::put(Object *o, int i){
	std::vector<Object *> &callframe = this->stack.front();
	if(i >= 0 && i < callframe.size()){
		callframe[i] = o;
	}
}

void Context::garbage_collect(void){
	for(std::vector<Object*> &v : stack){
		gc_sweep_vector(&v);
	}

	Object::gc_delete();
}


void Context::show_frame(void){
	std::vector<Object *> &callframe = this->stack.front();
	std::cout << "{\n";
	for(auto o : callframe){
		o->show();
	}
	std::cout << "}\n";
}

bool Context::ffi_load(std::string lib){
	void *handle = dlopen(strdup(lib.c_str()), RTLD_NOW);

	char *error = dlerror();
	if(error){
		std::cerr << "ffi_load: " << error << std::endl;
		return false;
	}

	ffi_handles[lib] = handle;
	
	return true;
}

/*
 * ffi symbols are in the format:
 * 	module::symbol
 */
bool Context::ffi_call_sym(std::string sym){
	std::string delim = "::";
	std::string module = sym.substr(0, sym.find(delim));
	std::string symbol = sym.substr(sym.find(delim)+delim.size(), sym.size());

	std::cout << "module=" << module << ", sym=" << symbol << std::endl;

	if(ffi_handles.count(module) < 1){
		std::cerr << "ffi_call_sym: cant find module: " << module << std::endl;
		return false;
	}

	void *handle = ffi_handles[module];
	void (*func)(std::vector<Object*>&)  = 
		(void (*)(std::vector<Object*>&))dlsym(handle, strdup(symbol.c_str()));

	char *error = dlerror();
	if(error){
		std::cerr << "ffi_call: " << error << std::endl;
		return false;
	}
	
	// calls the foreign function
	func(stack.front());

	return true;
}

