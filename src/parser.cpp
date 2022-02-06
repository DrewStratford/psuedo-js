#include <string>
#include <ctype.h>
#include <stdio.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include "ast.hpp"

#define MAKE_EXPR(EXP) std::shared_ptr<Expression>(EXP)
#define MAKE_STMT(STMT) std::shared_ptr<Statement>(STMT)
#define MAKE_ACCESS(EXP) std::shared_ptr<AccessorExp>(EXP)
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

	bool character(char *out){
		int i = index;
		if( i >= input.size())
			return false;
		if(input[i] == '\'')
			return false;
		if(input[i] == '"')
			return false;

		if(input[i] == '\\'){
			i++;
			if( i >= input.size())
				return false;
			switch(input[i]){
				case '0':
					*out = '\0';
					break;
				case 't':
					*out = '\t';
					break;
				case 'n':
					*out = '\n';
					break;
				case '\'':
					*out = '\'';
					break;
				case '"':
					*out = '"';
					break;
				case '\\':
					*out = '\\';
					break;
				default:
					return false;
			}
			index = i+1;
			return true;
		}

		index++;
		*out = input[i];
		return true;
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

bool parse_commasep_expr(ParseInfo *p, std::vector<ExprPtr > *exps);

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

ExprPtr  parse_expr0(ParseInfo *p);
ExprPtr  parse_expr1(ParseInfo *p);
ExprPtr  parse_expr2(ParseInfo *p);

ExprPtr  parse_Expression(ParseInfo *p);

ExprPtr  parse_ArithExpr(ParseInfo *p){
	ParseInfo working = *p;
	enum BinOp op;
	ExprPtr exp1 = nullptr;
	ExprPtr exp2 = nullptr;

	if((exp1 = parse_expr1(&working)) &&
	   parse_binop(&working, &op) &&
	  (exp2 = parse_Expression(&working))
	){

		*p = working;
		return MAKE_EXPR(new BinExp(op, exp1, exp2));
	}

	return nullptr;
}

ExprPtr  parse_parens(ParseInfo *p){
	ParseInfo working = *p;
	ExprPtr exp1 = nullptr;


	if(working.match("(") &&
	   (exp1 = parse_Expression(&working)) &&
	   working.match(")")
	){

		*p = working;
		return exp1;
	}

	return nullptr;
}

AccessPtr parse_field_accessor(ParseInfo *p){
	ParseInfo working = *p;
	std::string id;
	if( working.match(".") &&
		working.identifier(&id)
	  ){
		*p = working;
		return MAKE_ACCESS(new FieldAccessor(id));
	}
	return nullptr;
}

AccessPtr parse_array_accessor(ParseInfo *p){
	ParseInfo working = *p;
	ExprPtr  exp = nullptr;
	if( working.match("[") &&
		(exp = parse_Expression(&working)) && 
		working.match("]") 
	  ){
		*p = working;
		return MAKE_ACCESS(new ArrayAccessor(exp));
	}
	return nullptr;
}

AccessPtr parse_closure_call(ParseInfo *p){
	ParseInfo working = *p;
	ExprPtr clos = nullptr;
	std::vector<ExprPtr> args;

	if( working.match("(") &&
		parse_commasep_expr(&working, &args) &&
	  	working.match(")")
	){
		*p = working;
		return MAKE_ACCESS(new ClosureCallExp(nullptr, args));
	}

	return nullptr;
}

AccessPtr  parse_accessor(ParseInfo *p){
	AccessPtr out = nullptr;
	if((out = parse_field_accessor(p)) ||
		(out = parse_closure_call(p)) ||
		(out = parse_array_accessor(p)) ){
			return out;
		}
	return nullptr;
}

bool parse_accessors(ParseInfo *p, std::vector<AccessPtr > *accs){
	ParseInfo working = *p;
	AccessPtr  acc = nullptr;
	if(!(acc = parse_accessor(&working)) ) return false;
	
	accs->push_back(acc);

	while((acc = parse_accessor(&working)) ){
		accs->push_back(acc);
	}

	*p = working;
	return true;
}

//can't really fail
bool parse_commasep_expr(ParseInfo *p, std::vector<ExprPtr > *exps){
	ParseInfo working = *p;
	ExprPtr exp = nullptr;
	
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

ExprPtr  parse_ffi_call(ParseInfo *p){
	ParseInfo working = *p;
	std::string name;
	std::vector<ExprPtr > args;

	if( working.match("foreign") &&
		working.non_whitespace(&name)&&
	  	working.match("(") &&
		parse_commasep_expr(&working, &args) &&
	  	working.match(")")
	){
		*p = working;
		return MAKE_EXPR(new FFICallExp(name, args));
	}

	return nullptr;
}
ExprPtr  parse_closcall(ParseInfo *p){
	ParseInfo working = *p;
	ExprPtr clos = nullptr;
	std::vector<ExprPtr> args;

	if( working.match("call") &&
	  	working.match("[") &&
		(clos = parse_Expression(&working)) &&
	  	working.match("]") &&
	  	working.match("(") &&
		parse_commasep_expr(&working, &args) &&
	  	working.match(")")
	){
		*p = working;
		return MAKE_EXPR(new ClosureCallExp(clos, args));
	}

	return nullptr;
}

ExprPtr parse_funcall(ParseInfo *p){
	ParseInfo working = *p;
	std::string id;
	std::vector<ExprPtr > args;

	if( working.identifier(&id) &&
	  	working.match("(") &&
		parse_commasep_expr(&working, &args) &&
	  	working.match(")")
	){
		*p = working;
		return MAKE_EXPR(new CallExp(id, args));
	}

	return nullptr;
}

ExprPtr parse_fieldaccess(ParseInfo *p){
	ParseInfo working = *p;
	std::vector<AccessPtr > accs;
	ExprPtr exp = nullptr;

	if((exp = parse_expr0(&working)) && 
	  parse_accessors(&working, &accs)
	){
		for(auto& acc : accs){
			//exp = MAKE_EXPR(new GetFieldExp(acc, exp));
			acc->set_sub_expr(exp);
			exp = acc;
		}
		*p = working;
		return exp;
	}

	return nullptr;
}

ExprPtr parse_char(ParseInfo *p){
	char c;
	ParseInfo working = *p;

	if(working.match("'") && working.character(&c) && working.match("'")){
		*p = working;
		return MAKE_EXPR(new IntExp((int)c));
	}
	return nullptr;
}

ExprPtr  parse_int(ParseInfo *p){
	int i;
	ParseInfo working = *p;

	if(working.integer(&i)){
		*p = working;
		return MAKE_EXPR(new IntExp(i));
	}
	return nullptr;
}

ExprPtr  parse_var(ParseInfo *p){
	std::string s;
	ParseInfo working = *p;

	if(working.identifier(&s)){
		*p = working;
		return MAKE_EXPR( new VarExp(s) );
	}
	return nullptr;
}

ExprPtr  parse_unit(ParseInfo *p){
	ParseInfo working = *p;

	if(working.match("null")){
		*p = working;
		return MAKE_EXPR(new UnitExp());
	}
	return nullptr;
}

ExprPtr  parse_object(ParseInfo *p){
	ParseInfo working = *p;

	if(working.match("{}")){
		*p = working;
		return MAKE_EXPR( new ObjectExp() );
	}
	return nullptr;
}

ExprPtr parse_string(ParseInfo *p){
	ParseInfo working = *p;
	std::vector<char> chars;

	if(!working.match("\""))
		return nullptr;
	
	char c = '\0';
	while(working.character(&c)){
		chars.push_back(c);
	}

	if(!working.match("\""))
		return nullptr;
	
	*p = working;

	return MAKE_EXPR(new StringExp(chars));
}

ExprPtr  parse_vector(ParseInfo *p){
	ParseInfo working = *p;
	std::vector<ExprPtr> elems;

	if(working.match("[") &&
		parse_commasep_expr(&working, &elems) &&
		working.match("]")
	){
		*p = working;
		return std::shared_ptr<Expression>(new VectorExp(elems));
	}
	return nullptr;
}

std::shared_ptr<BlockStmt> parse_block(ParseInfo *p);

ExprPtr parse_closure(ParseInfo *p){
	ParseInfo working = *p;
	std::string id;
	std::vector<std::string> args;
	std::shared_ptr<BlockStmt> body;

	if(working.match("closure") &&
	   working.match("(") && 
	   parse_commasep_ident(&working, &args) &&
	   working.match(")") && 
	   (body = parse_block(&working))
	  ){
	  	*p = working;
		return MAKE_EXPR(new ClosureExp(args, body));
	}

	return nullptr;
}

ExprPtr  parse_expr0(ParseInfo *p){
	ExprPtr out = nullptr;
	if( out = parse_parens(p)) return out;
	else if( out = parse_ffi_call(p)) return out;
	//else if( out = parse_closcall(p)) return out;
	else if( out = parse_closure(p)) return out;
	//else if( out = parse_funcall(p)) return out;
	else if( out = parse_unit(p)) return out;
	else if( out = parse_char(p)) return out;
	else if( out = parse_int(p)) return out;
	else if( out = parse_var(p)) return out;
	else if( out = parse_object(p)) return out;
	else if( out = parse_string(p)) return out;
	else if( out = parse_vector(p)) return out;
	return out;
}

ExprPtr  parse_expr1(ParseInfo *p){
	ExprPtr out = nullptr;
	if( out = parse_fieldaccess(p)) return out;
	else if( out = parse_expr0(p)) return out;
	return out;
}

ExprPtr  parse_expr2(ParseInfo *p){
	ExprPtr out = nullptr;
	if( out = parse_ArithExpr(p)) return out;
	else if( out = parse_expr1(p)) return out;
	return out;
}

ExprPtr  parse_Expression(ParseInfo *p){
	ExprPtr out = nullptr;
	if( out = parse_expr2(p)) return out;
	return out;
}

StmtPtr parse_statement(ParseInfo *p);

StmtPtr parse_return(ParseInfo *p){
	ParseInfo working = *p;
	ExprPtr exp = nullptr;

	if(working.match("return") &&
	   (exp = parse_Expression(&working)) &&
	   working.match(";")
	  ){
	  	*p = working;
		return MAKE_STMT(new ReturnStmt(exp));
	  }

	  return nullptr;
}

StmtPtr parse_break(ParseInfo *p){
	ParseInfo working = *p;

	if(working.match("break") && working.match(";")){
		*p = working;
		return MAKE_STMT(new BreakStmt());
	  }
	  return nullptr;
}

StmtPtr parse_continue(ParseInfo *p){
	ParseInfo working = *p;

	if(working.match("continue") && working.match(";")){
		*p = working;
		return MAKE_STMT(new ContinueStmt());
	  }
	  return nullptr;
}

StmtPtr parse_let(ParseInfo *p){
	ParseInfo working = *p;
	std::string id;
	ExprPtr exp = nullptr;

	if(working.match("let") &&
	   working.identifier(&id) &&
	   working.match("=") &&
	   (exp = parse_Expression(&working)) &&
	   working.match(";")
	  ){
	  	*p = working;
		return MAKE_STMT(new DeclareStmt(exp, id));
	  }

	  return nullptr;
}

StmtPtr parse_bare_expression(ParseInfo *p){
	ParseInfo working = *p;
	ExprPtr expr0 = nullptr;
	if((expr0 = parse_Expression(&working)) &&
	   working.match(";")){
		*p = working;
		return MAKE_STMT(new ExpressionStmt(expr0));
	}
	return nullptr;
}

StmtPtr parse_setfield(ParseInfo *p){
	ParseInfo working = *p;
	std::vector<AccessPtr> accs;
	ExprPtr exp = nullptr;
	ExprPtr obj = nullptr;
	AccessPtr acc = nullptr;

	if((obj = parse_expr0(&working)) &&
	   parse_accessors(&working, &accs) &&
		working.match("=") &&
	   (exp = parse_Expression(&working)) &&
	   working.match(";")
	  ){
		acc = accs[0];
		acc->set_sub_expr(obj);
		for(int i = 1; i < accs.size(); i++){
			//exp = MAKE_EXPR(new GetFieldExp(acc, exp));
			auto a = accs[i];
			a->set_sub_expr(acc);
			acc = a;
		}
	  	*p = working;
		return MAKE_STMT(new SetFieldStmt(acc, exp));
	}

	return nullptr;
}

StmtPtr parse_assign(ParseInfo *p){
	ParseInfo working = *p;
	std::string id;
	ExprPtr exp = nullptr;

	if(working.identifier(&id) &&
	   working.match("=") &&
	   (exp = parse_Expression(&working)) &&
	   working.match(";")
	  ){
	  	*p = working;
		return MAKE_STMT( new AssignStmt(exp, id) );
	}

	return nullptr;
}

std::shared_ptr<BlockStmt> parse_block(ParseInfo *p){
	ParseInfo working = *p;
	std::string id;
	ExprPtr exp = nullptr;

	if(working.match("{")){
		std::vector<StmtPtr > stmts;
		StmtPtr stmt = nullptr;
		int i = 0;
		while(!working.peek("}") && (stmt = parse_statement(&working))){
			stmts.push_back(stmt);
			i++;
		}
		//putchar('\n');

		if(working.match("}")){
			*p = working;
			return std::shared_ptr<BlockStmt>(new BlockStmt(stmts));
		}
		//TODO DELETE statments
	}

	return nullptr;
}

StmtPtr parse_funcdef(ParseInfo *p){
	ParseInfo working = *p;
	std::string id;
	std::vector<std::string> args;
	std::shared_ptr<BlockStmt> body;

	//arguments not working at the moment
	if(working.match("fun") &&
	   working.identifier(&id) &&
	   working.match("(") && 
	   parse_commasep_ident(&working, &args) &&
	   working.match(")") && 
	   (body = parse_block(&working))
	  ){
	  	*p = working;
		return MAKE_STMT(new FunctionStmt(id, args, body));
	}

	return nullptr;
}

StmtPtr parse_while(ParseInfo *p){
	ParseInfo working = *p;
	ExprPtr exp;
	std::shared_ptr<BlockStmt> body;

	//arguments not working at the moment
	if(working.match("while") &&
	   (exp = parse_parens(&working)) &&
	   (body = parse_block(&working))
	  ){
	  	*p = working;
		return MAKE_STMT(new WhileStmt(exp, body));
	}

	return nullptr;
}

StmtPtr parse_if(ParseInfo *p){
	ParseInfo working = *p;
	ExprPtr exp;
	StmtPtr body;
	StmtPtr _else = nullptr;

	//arguments not working at the moment
	if(working.match("if") &&
	   (exp = parse_parens(&working)) &&
	   (body = parse_statement(&working))
	  ){
	  	*p = working;
		if(working.match("else") &&
			(_else = parse_statement(&working))){
	  		*p = working;
			return MAKE_STMT(new IfStmt(exp, body, _else));
		}
		return MAKE_STMT(new IfStmt(exp, body));
	}

	return nullptr;
}

StmtPtr parse_FFI_load(ParseInfo *p){
	ParseInfo working = *p;
	std::string ffi;

	if(working.match("#load") &&
	   working.non_whitespace(&ffi)
	  ){
	  	*p = working;
		return MAKE_STMT(new LoadFFIStmt(ffi));
	}

	return nullptr;
}

StmtPtr parse_statement(ParseInfo *p){
	StmtPtr  out = nullptr;

	if(out = parse_FFI_load(p)) return out;
	if(out = parse_funcdef(p)) return out;
	if(out = parse_if(p)) return out;
	if(out = parse_while(p)) return out;
	if(out = parse_return(p)) return out;
	if(out = parse_break(p)) return out;
	if(out = parse_continue(p)) return out;
	if(out = parse_setfield(p)) return out;
	if(out = parse_let(p)) return out;
	if(out = parse_assign(p)) return out;
	if(out = parse_block(p)) return out;
	if(out = parse_bare_expression(p)) return out;

	return nullptr;
}

/*
 * Creates closures for all the top level functions
 */
void create_closures(std::vector<Instruction> &ins, Dictionary *globals){

	// scan for label addresses
	for(int ins_ptr = 0; ins_ptr < ins.size(); ins_ptr++){
		Instruction instr = ins[ins_ptr];
		// HACKY, but we check for . so we don't include loop labels etc.
		if(instr.op == LABEL && instr.str[0] != '.'){
			globals->emplace(instr.str, ObjPtr(new Closure(ins_ptr)));
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

	
	StmtPtr stmt = parse_statement(&p);
	if(stmt == nullptr){
		puts("no parse");
		return 0;
	} 

	Context ctx = Context();
	CompilationState state;

	std::vector<Instruction> is= std::vector<Instruction>();
	auto c = std::map<std::string, int>();
	stmt->emit(state, c, is);

	int position = 0;
	for(auto i : is){
		std::cout << position << ":\t" << i << "\n";
		position++;
	}

	auto globals = new Dictionary();
	create_closures(is, globals);
	process_labels(is);

	std::cout << "===================================================\n";
	position = 0;
	for(auto i : is){
		std::cout << position << ":\t" << i << "\n";
		position++;
	}

	for(int ip = 0; ip >= 0 && ip < is.size(); ){
		auto i = is[ip];
		step_instruction(&ctx, i, &ip, globals);
	}

	std::cout << *globals;
	ctx.garbage_collect();
	return 0;
}
