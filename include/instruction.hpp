#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <string>
#include <vector>
#include <iostream>

#include "context.hpp"

enum OpCode{

	SHOW_FRAME,
	PUSH_FRAME,
	RET,
	DROP,


	LABEL, //NOP stores a label

	JMP_CND,
	JMP_LNK,
	JMP_LBL, // transformed to JMP during preprocessing
	JMP,
	JMP_CLOS,

	NEW_OBJ,
	NEW_VEC,
	NEW_CLOS,
	NEW_UNIT,
	NEW_STRING,

	CLOS_CAP, //puts stack address variable into closure

	LOAD_IMM_F,
	LOAD_IMM_I,

	//takes index into stack
	LOAD_STK,
	SET_STK,

	LOAD_GLB,
	SET_GLB,

	LOOKUP_S,
	INSERT_S,

	LOOKUP_V,
	INSERT_V,

	ADD,
	MIN,
	MUL,
	DIV,
	MOD,

	EQ,
	LT,
	LTE,
	GT,
	GTE,

	// Foreign function interface
	FFI_LOAD,	//loads lib
	FFI_CALL_SYM,	//calls based on sym
	FFI_CALL		//calls based on known func address
};


typedef struct Instruction{
	enum OpCode op;
	union{
		char * str;
		void * ptr;
		int index;
		int i;
		float f;
	};
	friend std::ostream& operator<<(std::ostream& os, const Instruction&);
} Instruction;

void step_instruction(Context *, Instruction, int*, Dictionary *);
Instruction show_frame(void);
Instruction push_frame(int);
Instruction ret(void);
Instruction drop(void);

Instruction label(const char *);
Instruction label(std::string);

Instruction jmp_cnd(int);
Instruction jmp_lnk(int);
Instruction jmp(int);
Instruction jmp_lbl(const char *);
Instruction jmp_lbl(std::string);
Instruction jmp_closure(void);

Instruction jmp(const char *);
Instruction jmp(std::string);

Instruction new_obj(void);
Instruction new_vec(void);
Instruction new_unit(void);
Instruction new_closure(int);
Instruction new_string(const char*);

Instruction closure_capture(int);

Instruction load_imm_f(float);
Instruction load_imm_i(int);
Instruction load_stk(int);
Instruction load_glb(const char *);

Instruction set_stk(int);
Instruction set_glb(const char *);

Instruction insert_s(const char *);
Instruction lookup_s(const char *);

Instruction insert_v(void);
Instruction lookup_v(void);

Instruction add(void);
Instruction min(void);
Instruction mul(void);
Instruction div(void);
Instruction mod(void);

Instruction eq(void);
Instruction lt(void);
Instruction lte(void);
Instruction gt(void);
Instruction gte(void);

Instruction ffi_load(const char *);
Instruction ffi_call_sym(const char *);
Instruction ffi_call(void *);

void process_labels(std::vector<Instruction> &ins);

enum ParamType { None, Int, Float, String, Index, Ptr };
const std::string instruction_names[]{
	"SHOW_FRAME", "PUSH_FRAME", "RET", "DROP",
	"LABEL",
	"JMP_CND", "JMP_LNK", "JMP_LBL", "JMP", "JMP_CLOS",
	"NEW_OBJ", "NEW_VEC", "NEW_CLOS", "NEW_UNIT", "NEW_STRING",
	"CLOS_CAP",
	"LOAD_IMM_F", "LOAD_IMM_I", "LOAD_STK", "SET_STK",
	"LOAD_GLB", "SET_GLB",
	"LOOKUP_S", "INSERT_S",
	"LOOKUP_V", "INSERT_V",
	"ADD", "MIN", "MUL", "DIV", "MOD",
	"EQ", "LT", "LTE", "GT", "GTE",
	"FFI_LOAD", "FFI_CALL_SYM", "FFI_CALL"
};
const ParamType instruction_types[]{
	None, None, None, None,
	String,
	Int, Int, String, Int, None,
	None, None, Int, None, String,
	Int,
	Float, Int, Int, Int,
	String, String,
	String, String,
	None, None,
	None, None, None, None, None,
	None, None, None, None, None,
	String, String, Ptr
};
#endif
