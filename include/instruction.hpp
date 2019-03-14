#ifndef INSTRUCTION_HPP
#define INSTRUCTION_HPP

#include <string>
#include <vector>

#include "context.hpp"

enum OpCode{

	SHOW_FRAME,
	PUSH_FRAME,
	RET,

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
} Instruction;

void step_instruction(Context *, Instruction, int*, Object *);
Instruction show_frame(void);
Instruction push_frame(int);
Instruction ret(void);

Instruction label(char *);

Instruction jmp_cnd(int);
Instruction jmp_lnk(int);
Instruction jmp(int);
Instruction jmp_lbl(char *);
Instruction jmp_closure(void);

Instruction jmp(char *);

Instruction new_obj(void);
Instruction new_vec(void);
Instruction new_unit(void);
Instruction new_closure(int);

Instruction closure_capture(int);

Instruction load_imm_f(float);
Instruction load_imm_i(int);
Instruction load_stk(int);
Instruction load_glb(char *);

Instruction set_stk(int);
Instruction set_glb(char *);

Instruction insert_s(char *);
Instruction lookup_s(char *);

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

Instruction ffi_load(char *);
Instruction ffi_call_sym(char *);
Instruction ffi_call(void *);

void process_labels(std::vector<Instruction> &ins);


#endif
