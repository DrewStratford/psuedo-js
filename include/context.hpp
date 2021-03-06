#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include "object.hpp"

class Context{
	private:
	/*
	 * Essentially a stack of call frames.
	 */
	std::list<std::vector<ObjPtr>> stack;

	//stores the return address from jmp_lnk
	std::vector<int> ret_stack;

	//Stores handles for FFI
	std::map<std::string, void*> ffi_handles;

	public:
	Context();
	~Context();

	void show_frame(void);

	ObjPtr get(int);
	void put(ObjPtr, int);

	void push(ObjPtr);
	ObjPtr pop(void);

	//takes the number of arguments being pushed
	void push_frame(int);
	void pop_frame(void);

	/*
	 * Pops the frame and then pushes the object
	 * onto the lower frame, returns the top of ret_stk.
	 */
	int ret(ObjPtr);
	void link(int lnk);

	void garbage_collect(void);

	bool ffi_load(std::string);
	bool ffi_call_sym(std::string);
	
};

#endif
