#include <string>
#include <ctype.h>
#include <stdio.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include "ast.hpp"

class ParseInfo{
	public:
	int index;
	std::string input;

	ParseInfo(std::string i){
		index = 0;
		input = i;
	}

	int skip_whitespace(void){
		int i = index;
		while(i< input.size() && isspace(input[i])){
			i++;
		}
		return i;
	}

	bool peek(std::string matching){
		
		int i = skip_whitespace();

		for(char c : matching){
			if(i >= input.size()) return false;
			if(c != input[i]) return false;

			i++;
		}
		return true;
	}
	bool match(std::string matching){
		
		int i = skip_whitespace();
		//printf("%d, %d\n", index, i);

		for(char c : matching){
			if(i >= input.size()) return false;
			if(c != input[i]) return false;

			i++;
		}
		index = i;
		return true;
	}

	//TODO: implement escape codes;
	bool character(char *out){
		*out = 0;

		int i = skip_whitespace();
		if(i+3 < (input.size()) &&
			input[i] == '\'' &&
			input[i+2] == '\''
		  ){

			*out = input[i+1];
			index = i+3;
			return true;
		}
		
		return false;
	}

	bool string(std::string *str){
		int i = skip_whitespace();

		if( i < input.size() && input[i] == '"'){
			i++;
			int start = i;
			while(i < input.size()){
				if(input[i] == '"'){
						*str = input.substr(start, i-start);
						index = i+1;
						return true;
				}
				i++;
			}
		}
		return false;
		
	}

	bool integer(int *out){
		*out = 0;
		bool found = false;
		int i = skip_whitespace();

		while(i< input.size() && isdigit(input[i])){
			*out *= 10;
			*out += input[i] - 48;
			found = true;
			i++;
		}
		
		if(found) index = i;
		return found;
	}

	bool identifier(std::string *out){
		bool found = false;

		int i = skip_whitespace();
		int start = i;

		while(i< input.size() && isalpha(input[i])){
			found = true;
			i++;
		}
		
		if(found) {
			index = i;
			*out = input.substr(start, index-start);
		}
		return found;
	}

	bool non_whitespace(std::string *out){
		bool found = false;

		int i = skip_whitespace();
		int start = i;

		while(i< input.size() && 
				!isspace(input[i]) &&
				// There has to be a better way
				// to do this.
				input[i] != '(' && 
				input[i] != ')' && 
				input[i] != '[' && 
				input[i] != ']'
				){
			found = true;
			i++;
		}
		
		if(found) {
			index = i;
			*out = input.substr(start, index-start);
		}
		return found;
	}

	
};

std::string op_str[] = 
	{ "==", "<=", "<", ">=", ">", "+", "-", "*", "/", "%"};
enum BinOp op_enum[] = 
	{EQ_OP, LTE_OP, LT_OP, GTE_OP, GT_OP, ADD_OP, MIN_OP, 
	MUL_OP, DIV_OP, MOD_OP};
bool parse_binop(ParseInfo *p, enum BinOp *b){

	for(int i = 0; i < sizeof(op_str) / sizeof(op_str[1]); i++){
		if( p->match(op_str[i]) ){
			*b = op_enum[i];
			return true;
		}
	}
	return false;
}

Expression * parse_expr0(ParseInfo *p);
Expression * parse_expr1(ParseInfo *p);
Expression * parse_expr2(ParseInfo *p);

Expression * parse_Expression(ParseInfo *p);

Expression * parse_ArithExpr(ParseInfo *p){
	ParseInfo working = *p;
	enum BinOp op;
	Expression *exp1 = nullptr;
	Expression *exp2 = nullptr;

	if((exp1 = parse_expr1(&working)) &&
	   parse_binop(&working, &op) &&
	  (exp2 = parse_Expression(&working))
	){

		*p = working;
		return new BinExp(op, exp1, exp2);
	}

	if(exp1 == nullptr) delete exp1;
	if(exp2 == nullptr) delete exp2;
	return nullptr;
}

Expression * parse_parens(ParseInfo *p){
	ParseInfo working = *p;
	Expression *exp1 = nullptr;


	if(working.match("(") &&
	   (exp1 = parse_Expression(&working)) &&
	   working.match(")")
	){

		*p = working;
		return exp1;
	}

	if(exp1 == nullptr) delete exp1;
	return nullptr;
}

FieldAccessor* parse_field_accessor(ParseInfo *p){
	ParseInfo working = *p;
	std::string id;
	if( working.match(".") &&
		working.identifier(&id)
	  ){
		*p = working;
		return new FieldAccessor(id);
	}
	return nullptr;
}

ArrayAccessor *parse_array_accessor(ParseInfo *p){
	ParseInfo working = *p;
	Expression * exp = nullptr;
	if( working.match("[") &&
		(exp = parse_Expression(&working)) && 
		working.match("]") 
	  ){
		*p = working;
		return new ArrayAccessor(exp);
	}
	return nullptr;
}

AccessorExp * parse_accessor(ParseInfo *p){
	AccessorExp *out = nullptr;
	if((out = parse_field_accessor(p)) ||
		(out = parse_array_accessor(p)) ){
			return out;
		}
	return nullptr;
}

bool parse_accessors(ParseInfo *p, std::vector<AccessorExp *> *accs){
	ParseInfo working = *p;
	AccessorExp * acc = nullptr;
	if(!(acc = parse_accessor(&working)) ) return false;
	
	accs->push_back(acc);

	while((acc = parse_accessor(&working)) ){
		accs->push_back(acc);
	}

	*p = working;
	return true;
}

//can't really fail
bool parse_commasep_expr(ParseInfo *p, std::vector<Expression *> *exps){
	ParseInfo working = *p;
	Expression *exp = nullptr;
	
	if(!(exp = parse_Expression(&working))) return true;
	exps->push_back(exp);

	*p = working;

	while(working.match(",") &&
	      (exp = parse_Expression(&working))
		 ){
		exps->push_back(exp);
		*p = working;
	}
	return true;
}

//can't really fail
bool parse_commasep_ident(ParseInfo *p, std::vector<std::string> *ids){
	ParseInfo working = *p;
	std::string id;
	
	//if can't find any return early
	if(!working.identifier(&id)) return true;
	ids->push_back(id);

	*p = working;

	while(working.match(",") &&
	      working.identifier(&id)
		 ){
		ids->push_back(id);
		*p = working;
	}
	return true;
}

Expression * parse_ffi_call(ParseInfo *p){
	ParseInfo working = *p;
	std::string name;
	std::vector<Expression *> args;

	if( working.match("foreign") &&
	  	//working.match("[") &&
		working.non_whitespace(&name)&&
	  	//working.match("]") &&
	  	working.match("(") &&
		parse_commasep_expr(&working, &args) &&
	  	working.match(")")
	){
		*p = working;
		return new FFICallExp(name, args);
	}

	return nullptr;
}
Expression * parse_closcall(ParseInfo *p){
	ParseInfo working = *p;
	Expression *clos = nullptr;
	std::vector<Expression *> args;

	if( working.match("call") &&
	  	working.match("[") &&
		(clos = parse_Expression(&working)) &&
	  	working.match("]") &&
	  	working.match("(") &&
		parse_commasep_expr(&working, &args) &&
	  	working.match(")")
	){
		*p = working;
		return new ClosureCallExp(clos, args);
	}

	return nullptr;
}

Expression * parse_funcall(ParseInfo *p){
	ParseInfo working = *p;
	std::string id;
	std::vector<Expression *> args;

	if( working.identifier(&id) &&
	  	working.match("(") &&
		parse_commasep_expr(&working, &args) &&
	  	working.match(")")
	){
		*p = working;
		return new CallExp(id, args);
	}

	//TODO delete stuff properly
	return nullptr;
}

Expression * parse_fieldaccess(ParseInfo *p){
	ParseInfo working = *p;
	std::vector<AccessorExp *> accs;
	Expression *exp = nullptr;

	if((exp = parse_expr0(&working)) && 
	  parse_accessors(&working, &accs)
	){
		for(auto& acc : accs){
			exp = new GetFieldExp(acc, exp);
		}
		*p = working;
		return exp;
	}

	if(exp == nullptr) delete exp;
	return nullptr;
}

Expression * parse_char(ParseInfo *p){
	int c;
	ParseInfo working = *p;

	if(working.character((char *)&c)){
		*p = working;
		return new IntExp(c);
	}
	return nullptr;
}
Expression * parse_int(ParseInfo *p){
	int i;
	ParseInfo working = *p;

	if(working.integer(&i)){
		*p = working;
		return new IntExp(i);
	}
	return nullptr;
}

Expression * parse_var(ParseInfo *p){
	std::string s;
	ParseInfo working = *p;

	if(working.identifier(&s)){
		*p = working;
		return new VarExp(strdup(s.c_str()));
	}
	return nullptr;
}

Expression * parse_unit(ParseInfo *p){
	ParseInfo working = *p;

	if(working.match("null")){
		*p = working;
		return new UnitExp();
	}
	return nullptr;
}

Expression * parse_object(ParseInfo *p){
	ParseInfo working = *p;

	if(working.match("{}")){
		*p = working;
		return new ObjectExp();
	}
	return nullptr;
}

Expression * parse_string(ParseInfo *p){
	ParseInfo working = *p;
	std::string str;
	std::vector<Expression *> chars;

	if(working.string(&str)){
		*p = working;
		for(auto c : str){
			chars.push_back( new IntExp(c));
		}
		return new VectorExp(chars);
	}
	return nullptr;
}

Expression * parse_vector(ParseInfo *p){
	ParseInfo working = *p;
	std::vector<Expression *> elems;

	if(working.match("[") &&
		parse_commasep_expr(&working, &elems) &&
		working.match("]")
	){
		*p = working;
		return new VectorExp(elems);
	}
	return nullptr;
}

BlockStmt *parse_block(ParseInfo *p);

Expression *parse_closure(ParseInfo *p){
	ParseInfo working = *p;
	std::string id;
	std::vector<std::string> args;
	BlockStmt *body;

	if(working.match("closure") &&
	   working.match("(") && 
	   parse_commasep_ident(&working, &args) &&
	   working.match(")") && 
	   (body = parse_block(&working))
	  ){
	  	*p = working;
		return new ClosureExp(args, body);
	}

	return nullptr;
}

Expression * parse_expr0(ParseInfo *p){
	Expression *out = nullptr;
	if( out = parse_parens(p)) return out;
	else if( out = parse_ffi_call(p)) return out;
	else if( out = parse_closcall(p)) return out;
	else if( out = parse_closure(p)) return out;
	else if( out = parse_funcall(p)) return out;
	else if( out = parse_unit(p)) return out;
	else if( out = parse_char(p)) return out;
	else if( out = parse_int(p)) return out;
	else if( out = parse_var(p)) return out;
	else if( out = parse_object(p)) return out;
	else if( out = parse_string(p)) return out;
	else if( out = parse_vector(p)) return out;
	return out;
}

Expression * parse_expr1(ParseInfo *p){
	Expression *out = nullptr;
	if( out = parse_fieldaccess(p)) return out;
	else if( out = parse_expr0(p)) return out;
	return out;
}

Expression * parse_expr2(ParseInfo *p){
	Expression *out = nullptr;
	if( out = parse_ArithExpr(p)) return out;
	else if( out = parse_expr1(p)) return out;
	return out;
}

Expression * parse_Expression(ParseInfo *p){
	Expression *out = nullptr;
	if( out = parse_expr2(p)) return out;
	return out;
}

Statement *parse_statement(ParseInfo *p);

Statement *parse_return(ParseInfo *p){
	ParseInfo working = *p;
	std::string id;
	Expression *exp = nullptr;

	if(working.match("return") &&
	   (exp = parse_Expression(&working)) &&
	   working.match(";")
	  ){
	  	*p = working;
		return new ReturnStmt(exp);
	  }

	  if(exp != nullptr) delete exp;
	  return nullptr;
}
Statement *parse_let(ParseInfo *p){
	ParseInfo working = *p;
	std::string id;
	Expression *exp = nullptr;

	if(working.match("let") &&
	   working.identifier(&id) &&
	   working.match("=") &&
	   (exp = parse_Expression(&working)) &&
	   working.match(";")
	  ){
	  	*p = working;
		return new DeclareStmt(exp, strdup(id.c_str()));
	  }

	  if(exp != nullptr) delete exp;
	  return nullptr;
}

Statement *parse_setfield(ParseInfo *p){
	ParseInfo working = *p;
	std::vector<AccessorExp *> accs;
	Expression *exp = nullptr;
	Expression *obj = nullptr;

	if((obj = parse_expr0(&working)) &&
	   parse_accessors(&working, &accs) &&
		working.match("=") &&
	   (exp = parse_Expression(&working)) &&
	   working.match(";")
	  ){
		AccessorExp *acc = nullptr;
		for(int i = 0; i < accs.size()-1; i++){
			acc = accs[i];
			obj = new GetFieldExp(acc, obj);
		}
		acc = accs[accs.size()-1];
	  	*p = working;
		//return new SetFieldStmt(strdup(field.c_str()), obj, exp);
		return new SetFieldStmt(acc, obj, exp);
	}

	if(exp != nullptr) delete exp;
	return nullptr;
}

Statement *parse_assign(ParseInfo *p){
	ParseInfo working = *p;
	std::string id;
	Expression *exp = nullptr;

	if(working.identifier(&id) &&
	   working.match("=") &&
	   (exp = parse_Expression(&working)) &&
	   working.match(";")
	  ){
	  	*p = working;
		return new AssignStmt(exp, strdup(id.c_str()));
	}

	if(exp != nullptr) delete exp;
	return nullptr;
}

BlockStmt *parse_block(ParseInfo *p){
	ParseInfo working = *p;
	std::string id;
	Expression *exp = nullptr;

	if(working.match("{")){
		std::vector<Statement *> stmts;
		Statement *stmt = nullptr;
		int i = 0;
		while(!working.peek("}") && (stmt = parse_statement(&working))){
			stmts.push_back(stmt);
			i++;
		}
		//putchar('\n');

		if(working.match("}")){
			*p = working;
			return new BlockStmt(stmts);
		}
		//TODO DELETE statments
	}

	return nullptr;
}

Statement *parse_funcdef(ParseInfo *p){
	ParseInfo working = *p;
	std::string id;
	std::vector<std::string> args;
	BlockStmt *body;

	//arguments not working at the moment
	if(working.match("fun") &&
	   working.identifier(&id) &&
	   working.match("(") && 
	   parse_commasep_ident(&working, &args) &&
	   working.match(")") && 
	   (body = parse_block(&working))
	  ){
	  	*p = working;
		return new FunctionStmt(strdup(id.c_str()), args, body);
	}

	return nullptr;
}

Statement *parse_while(ParseInfo *p){
	ParseInfo working = *p;
	Expression *exp;
	BlockStmt *body;

	//arguments not working at the moment
	if(working.match("while") &&
	   (exp = parse_parens(&working)) &&
	   (body = parse_block(&working))
	  ){
	  	*p = working;
		return new WhileStmt(exp, body);
	}

	return nullptr;
}

Statement *parse_if(ParseInfo *p){
	ParseInfo working = *p;
	Expression *exp;
	Statement *body;
	Statement *_else = nullptr;

	//arguments not working at the moment
	if(working.match("if") &&
	   (exp = parse_parens(&working)) &&
	   (body = parse_statement(&working))
	  ){
	  	*p = working;
		if(working.match("else") &&
			(_else = parse_statement(&working))){
	  		*p = working;
			return new IfStmt(exp, body, _else);
		}
		return new IfStmt(exp, body);
	}

	return nullptr;
}

Statement *parse_FFI_load(ParseInfo *p){
	ParseInfo working = *p;
	std::string ffi;

	if(working.match("#load") &&
	   working.non_whitespace(&ffi)
	  ){
	  	*p = working;
		return new LoadFFIStmt(ffi);
	}

	return nullptr;
}

Statement *parse_statement(ParseInfo *p){
	Statement * out = nullptr;

	if(out = parse_FFI_load(p)) return out;
	if(out = parse_funcdef(p)) return out;
	if(out = parse_if(p)) return out;
	if(out = parse_while(p)) return out;
	if(out = parse_return(p)) return out;
	if(out = parse_setfield(p)) return out;
	if(out = parse_let(p)) return out;
	if(out = parse_assign(p)) return out;
	if(out = parse_block(p)) return out;

	return nullptr;
}

/*
 * Creates closures for all the top level functions
 */
void create_closures(std::vector<Instruction> &ins, Object *globals){

	// scan for label addresses
	for(int ins_ptr = 0; ins_ptr < ins.size(); ins_ptr++){
		Instruction instr = ins[ins_ptr];
		if(instr.op == LABEL){
			globals->set(instr.str, new Object(new Closure(ins_ptr)));
		}
	}
}

int main(int argc, char **argv){
	if(argc < 2) return 0;

	std::stringstream input;
	std::ifstream file;
	file.open(argv[1]);
	input << file.rdbuf();

	std::string s = input.str();

	ParseInfo p = ParseInfo(s);

	
	Statement *stmt = parse_statement(&p);
	if(stmt == nullptr){
		puts("no parse");
		return 0;
	} 

	Context ctx = Context();

	std::vector<Instruction> is= std::vector<Instruction>();
	auto c = std::map<std::string, int>();
	stmt->emit(c, is);

	auto globals = new Object(OBJECT);
	//create_closures(is, globals);
	process_labels(is);

	for(int ip = 0; ip >= 0 && ip < is.size(); ){
		auto i = is[ip];
		step_instruction(&ctx, i, &ip, globals);
	}

	globals->show();
	ctx.garbage_collect();
	return 0;
}
