#include "instruction.hpp"

#include <iostream>
#include <cstring>

void step_instruction(Context * ctxt, 
					Instruction i, 
					int *ip, 
					Dictionary *globals){
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
		case DROP:
			ctxt->pop();
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
			ObjPtr a = ctxt->pop();
			ObjPtr b = ObjPtr(1);
			if(a.as_i() == 1) step = i.i;
			}
			break;
		case JMP_CLOS:
			// Pops a closure from the stack
			// then jmps and links
			{
			ObjPtr c = ctxt->pop();
			Closure *clos = nullptr;
			if(clos = c.as_c()){
				//push captured vars (all fully eval'd)
				for(auto obj : clos->env){
					ctxt->push(obj);
				}
				ctxt->link((*ip) + 1);
				//convert absolute to relative
				//address;
				int addr = clos->get_func() - *ip;

				step = addr; 
			}
			}
			break;
		case NEW_OBJ:
			ctxt->push(ObjPtr(new Dictionary()));
			break;
		case NEW_VEC:
			ctxt->push(ObjPtr(new ArrayList()));
			break;
		case NEW_UNIT:
			ctxt->push(ObjPtr());
			break;
		case NEW_STRING:
			ctxt->push(ObjPtr(new StringObject(std::string(i.str))));
			break;
		case NEW_CLOS:
			{
			Closure *c = new Closure(i.i);
			ctxt->push(ObjPtr(c));
			}
			break;
		case CLOS_CAP:
			{
			ObjPtr clos = ctxt->pop();
			ObjPtr captured = ctxt->get(i.index);
			Closure *closure = nullptr;
			if(closure = clos.as_c()){
				closure->push_var(captured);
			}
			ctxt->push(clos);
			}
			break;
		case LOAD_IMM_F:
			ctxt->push(ObjPtr(i.f));
			break;
		case LOAD_IMM_I:
			ctxt->push(ObjPtr(i.i));
			break;
		case LOAD_STK:
			ctxt->push(ctxt->get(i.index));
			break;
		case LOAD_GLB:
			ctxt->push(globals->at(i.str));
			break;
		case SET_STK:
			ctxt->put(ctxt->pop(), i.index);
			break;
		case SET_GLB:
			globals->insert_or_assign(i.str, ctxt->pop());
			break;
		case LOOKUP_S:
			{
			//TODO
			ObjPtr o = ctxt->pop();
			Dictionary *dict = nullptr;
			if(dict = o.as_dict())
				ctxt->push(dict->at(i.str));
			}
			break;
		case INSERT_S:
			{
			ObjPtr obj = ctxt->pop();
			ObjPtr b = ctxt->pop();
			Dictionary *dict = nullptr;
			if(dict = obj.as_dict())
				//TODO: We erase here because emplace doesn't overwrite,
				// This could probably be improved by using iterators.
				dict->erase(i.str);
				dict->emplace(i.str, b);
			}
			break;
		case LOOKUP_V:
			{
			ObjPtr index = ctxt->pop();
			ObjPtr vec = ctxt->pop();
			ArrayList *arr = nullptr;
			if(arr = vec.as_arr()){
				ObjPtr out = arr->at(index.as_i());
				ctxt->push(out);
			}
			}
			break;
		case INSERT_V:
			{
			ObjPtr index = ctxt->pop();
			ObjPtr vec = ctxt->pop();
			ObjPtr adding = ctxt->pop();
			ArrayList *arr = nullptr;
			if(arr = vec.as_arr())
				arr->at(index.as_i()) = adding;
			}
			break;

		case ADD:
			{
			ObjPtr a = ctxt->pop();
			ObjPtr b = ctxt->pop();
			ctxt->push(add(b, a));
			}
			break;
		case MIN:
			{
			ObjPtr a = ctxt->pop();
			ObjPtr b = ctxt->pop();
			ctxt->push(sub(b, a));
			}
			break;
		case MUL:
			{
			ObjPtr a = ctxt->pop();
			ObjPtr b = ctxt->pop();
			ctxt->push(mul(b, a));
			}
			break;
		case DIV:
			{
			ObjPtr a = ctxt->pop();
			ObjPtr b = ctxt->pop();
			ctxt->push(div(b, a));
			}
			break;
		case MOD:
			{
			ObjPtr a = ctxt->pop();
			ObjPtr b = ctxt->pop();
			ctxt->push(mod(b, a));
			}
			break;
		case LT:
			{
			ObjPtr a = ctxt->pop();
			ObjPtr b = ctxt->pop();
			ctxt->push(lt(b, a));
			}
			break;
		case LTE:
			{
			ObjPtr a = ctxt->pop();
			ObjPtr b = ctxt->pop();
			ctxt->push(lte(b, a));
			}
			break;
		case GT:
			{
			ObjPtr a = ctxt->pop();
			ObjPtr b = ctxt->pop();
			ctxt->push(gt(b, a));
			}
			break;
		case GTE:
			{
			ObjPtr a = ctxt->pop();
			ObjPtr b = ctxt->pop();
			ctxt->push(gte(b, a));
			}
			break;
		case EQ:
			{
			ObjPtr a = ctxt->pop();
			ObjPtr b = ctxt->pop();

			//TODO: doesn't work for anything other than ints.
			ctxt->push(a.as_i() == b.as_i());
			}
			break;

		case FFI_LOAD:
			ctxt->ffi_load(i.str);
			break;
		case FFI_CALL_SYM:
			//technically doesn't do anything
			//but we still need to link this call
			ctxt->link((*ip) + 1);

			ctxt->ffi_call_sym(i.str);
			ctxt->ret(ctxt->pop());
			break;
		case FFI_CALL:
			puts("TODO: implement ffi_call");
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
Instruction drop(void){
	return {.op = DROP};
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

Instruction jmp_lbl(const char *c){
	Instruction out;
	out.op = JMP_LBL;
	out.str = strdup(c);
	return out;
}

Instruction jmp_closure(void){
	Instruction out;
	out.op = JMP_CLOS;
	return out;
}

Instruction label(const char *c){
	Instruction out;
	out.op = LABEL;
	out.str = strdup(c);
	return out;
}

Instruction new_obj(void){
	return {.op = NEW_OBJ};
}
Instruction new_vec(void){
	return {.op = NEW_VEC};
}

Instruction new_unit(void){
	return {.op = NEW_UNIT};
}

Instruction new_string(const char* str){
	Instruction out;
	out.op = NEW_STRING;
	out.str = strdup(str);
	return out;
}

Instruction new_closure(int ip){
	Instruction out;
	out.op = NEW_CLOS;
	out.i = ip;
	return out;
}

Instruction closure_capture(int addr){
	Instruction out;
	out.op = CLOS_CAP;
	out.index = addr;
	return out;
}

Instruction call_closure(void){
	return {.op = JMP_CLOS};
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

Instruction load_glb(const char *c){
	Instruction out;
	out.op = LOAD_GLB;
	out.str = strdup(c);
	return out;
}


Instruction set_stk(int i){
	Instruction out;
	out.op = SET_STK;
	out.index =i;
	return out;
}

Instruction set_glb(const char *c){
	Instruction out;
	out.op = SET_GLB;
	out.str = strdup(c);
	return out;
}

Instruction insert_s(const char *c){
	Instruction out;
	out.op = INSERT_S;
	out.str = strdup(c);
	return out;
}

Instruction lookup_s(const char *c){
	Instruction out;
	out.op = LOOKUP_S;
	out.str = strdup(c);
	return out;
}

Instruction ffi_load(const char *c){
	Instruction out;
	out.op = FFI_LOAD;
	out.str = strdup(c);
	return out;
}

Instruction ffi_call_sym(const char *c){
	Instruction out;
	out.op = FFI_CALL_SYM;
	out.str = strdup(c);
	return out;
}

Instruction ffi_call(void *func_ptr){
	Instruction out;
	out.op = FFI_LOAD;
	out.ptr = func_ptr;
	return out;
}

Instruction lookup_v(void){ return {.op = LOOKUP_V}; }
Instruction insert_v(void){ return {.op = INSERT_V}; }

Instruction add(void){ return {.op = ADD}; }
Instruction min(void){ return {.op = MIN}; }
Instruction mul(void){ return {.op = MUL}; }
Instruction div(void){ return {.op = DIV}; }
Instruction mod(void){ return {.op = MOD}; }
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
			free(instr.str);
			ins[ins_ptr].op = JMP_LNK;
			ins[ins_ptr].i = relative;
		}
	}

	// changes closures from captured count -> absolute addr
	// Note each closure's op codes are compiled + 3 + captured_vars
	// from the new_closure.
	for(int ins_ptr = 0; ins_ptr < ins.size(); ins_ptr++){
		Instruction instr = ins[ins_ptr];
		if(instr.op == NEW_CLOS){
			int absolute = ins_ptr + 2 + instr.index;
			ins[ins_ptr].op = NEW_CLOS;
			ins[ins_ptr].index = absolute;
		}
	}
}

std::ostream& operator<<(std::ostream& os, const Instruction& i){
	os << instruction_names[i.op];
	switch(instruction_types[i.op]){
		case Int: os << ": " << i.i; break;
		case Float: os << ": " << i.f; break;
		case String: os << ": " << i.str; break;
		case Ptr: os << ": " << i.ptr; break;
		default: break;
	}

	return os;
}
