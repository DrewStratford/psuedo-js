#include "instruction.hpp"

#include <iostream>

void step_instruction(Context * ctxt, 
					Instruction i, 
					int *ip, 
					Object *globals){
	int step = 1;
	switch(i.op){
		case LABEL: // essentially a nop
			break;
		case SHOW_FRAME:
			ctxt->show_frame();
			break;
		case PUSH_FRAME:
			ctxt->push_frame(i.i);
			break;
		case RET:
			*ip = ctxt->ret(ctxt->pop());
			step = 0;
			break;
		case JMP:
			step = i.i;
			break;
		case JMP_LNK:
			ctxt->link((*ip) + 1);
			step = i.i;
			break;
		case JMP_CND:
			{
			Object *a = ctxt->pop();
			Object *b = new Object(1);
			if(a->equals(b)) step = i.i;
			}
			break;

		case NEW_OBJ:
			ctxt->push(new Object(OBJECT));
			break;
		case NEW_VEC:
			ctxt->push(new Object(VECTOR));
			break;

		case LOAD_IMM_F:
			ctxt->push(new Object(i.f));
			break;
		case LOAD_IMM_I:
			ctxt->push(new Object(i.i));
			break;
		case LOAD_STK:
			ctxt->push(ctxt->get(i.index));
			break;
		case LOAD_GLB:
			ctxt->push(globals->lookup(i.str));
			break;
		case SET_STK:
			ctxt->put(ctxt->pop(), i.index);
			break;
		case SET_GLB:
			globals->set(i.str, ctxt->pop());
			break;
		case LOOKUP_S:
			{
			Object *o = ctxt->pop();
			ctxt->push(o->lookup(i.str));
			}
			break;
		case INSERT_S:
			{
			Object *obj = ctxt->pop();
			Object *b = ctxt->pop();
			obj->set(i.str, b);
			}
			break;

		case ADD:
			{
			Object *a = ctxt->pop();
			Object *b = ctxt->pop();
			ctxt->push(add(b, a));
			}
			break;
		case MIN:
			{
			Object *a = ctxt->pop();
			Object *b = ctxt->pop();
			ctxt->push(sub(b, a));
			}
			break;
		case MUL:
			{
			Object *a = ctxt->pop();
			Object *b = ctxt->pop();
			ctxt->push(mul(b, a));
			}
			break;
		case DIV:
			{
			Object *a = ctxt->pop();
			Object *b = ctxt->pop();
			ctxt->push(div(b, a));
			}
			break;
		case LT:
			{
			Object *a = ctxt->pop();
			Object *b = ctxt->pop();
			ctxt->push(lt(b, a));
			}
			break;
		case LTE:
			{
			Object *a = ctxt->pop();
			Object *b = ctxt->pop();
			ctxt->push(lte(b, a));
			}
			break;
		case GT:
			{
			Object *a = ctxt->pop();
			Object *b = ctxt->pop();
			ctxt->push(gt(b, a));
			}
			break;
		case GTE:
			{
			Object *a = ctxt->pop();
			Object *b = ctxt->pop();
			ctxt->push(gte(b, a));
			}
			break;
		case EQ:
			{
			Object *a = ctxt->pop();
			Object *b = ctxt->pop();
			bool c = b->equals(a);
			ctxt->push(new Object(c));
			}
			break;
		default:
			std::cout << "error unimplemented op code" << std::endl;
			break;

	}
	*ip += step;
}

Instruction show_frame(void){
	return {.op = SHOW_FRAME};
}
Instruction push_frame(int i){
	Instruction out;
	out.op = PUSH_FRAME;
	out.i =i;
	return out;
}
Instruction ret(void){
	return {.op = RET};
}

Instruction jmp_cnd(int i){
	Instruction out;
	out.op = JMP_CND;
	out.i =i;
	return out;
}
Instruction jmp(int i){
	Instruction out;
	out.op = JMP;
	out.i =i;
	return out;
}

Instruction jmp_lnk(int i){
	Instruction out;
	out.op = JMP_LNK;
	out.i =i;
	return out;
}

Instruction jmp_lbl(char *c){
	Instruction out;
	out.op = JMP_LBL;
	out.str =c;
	return out;
}

Instruction label(char *c){
	Instruction out;
	out.op = LABEL;
	out.str =c;
	return out;
}

Instruction new_obj(void){
	return {.op = NEW_OBJ};
}
Instruction new_vec(void){
	return {.op = NEW_VEC};
}

Instruction load_imm_f(float f){
	Instruction out;
	out.op = LOAD_IMM_F;
	out.f =f;
	return out;
}

Instruction load_imm_i(int i){
	Instruction out;
	out.op = LOAD_IMM_I;
	out.i =i;
	return out;
}

Instruction load_stk(int i){
	Instruction out;
	out.op = LOAD_STK;
	out.index =i;
	return out;
}

Instruction load_glb(char *c){
	Instruction out;
	out.op = LOAD_GLB;
	out.str =c;
	return out;
}


Instruction set_stk(int i){
	Instruction out;
	out.op = SET_STK;
	out.index =i;
	return out;
}

Instruction set_glb(char *c){
	Instruction out;
	out.op = SET_GLB;
	out.str =c;
	return out;
}

Instruction insert_s(char *c){
	Instruction out;
	out.op = INSERT_S;
	out.str =c;
	return out;
}

Instruction lookup_s(char *c){
	Instruction out;
	out.op = LOOKUP_S;
	out.str = c;
	return out;
}

Instruction add(void){ return {.op = ADD}; }
Instruction min(void){ return {.op = MIN}; }
Instruction mul(void){ return {.op = MUL}; }
Instruction div(void){ return {.op = DIV}; }
Instruction eq(void){ return {.op = EQ}; }

Instruction lt(void){ return {.op = LT}; }
Instruction lte(void){ return {.op = LTE}; }
Instruction gt(void){ return {.op = GT}; }
Instruction gte(void){ return {.op = GTE}; }

void process_labels(std::vector<Instruction> &ins){
	std::map<std::string, int> lookup_table;

	// scan for label addresses
	for(int ins_ptr = 0; ins_ptr < ins.size(); ins_ptr++){
		Instruction instr = ins[ins_ptr];
		if(instr.op == LABEL){
			lookup_table[instr.str] = ins_ptr;
		}
	}

	// changes jmp_lbl to the appropriate jmp_lnk rel
	for(int ins_ptr = 0; ins_ptr < ins.size(); ins_ptr++){
		Instruction instr = ins[ins_ptr];
		if(instr.op == JMP_LBL){
			int absolute = lookup_table[instr.str];
			int relative = absolute - ins_ptr;
			ins[ins_ptr].op = JMP_LNK;
			ins[ins_ptr].i = relative;
		}
	}
}
