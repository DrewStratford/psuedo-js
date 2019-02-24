#include "ast.hpp"
#include <iostream>


void Expression::get_variables(std::set<std::string> &vars){
}

IntExp::IntExp(int i){
	this->i = i;
}

void IntExp::emit(std::map<std::string, int> &context,
				  std::vector<Instruction> &is){
	is.push_back(load_imm_i(this->i));
}

FloatExp::FloatExp(float f){
	this->f = f;
}

void FloatExp::emit(std::map<std::string, int> &context, 
					std::vector<Instruction> &is){
	is.push_back(load_imm_f(this->f));
}

ObjectExp::ObjectExp(){
}

void ObjectExp::emit(std::map<std::string, int> &context,
					 std::vector<Instruction> &is){
	is.push_back( new_obj() );
}

VarExp::VarExp(char *s){
	this->var = s;
}

void VarExp::emit(std::map<std::string, int> &context,
				  std::vector<Instruction> &is){
	if(context.count(this->var) != 0){
		int &stack_pos = context.at(this->var);
		is.push_back(load_stk(stack_pos));
	} else{
		char* s = this->var;
		is.push_back(load_glb(s));
	}
}

void VarExp::get_variables(std::set<std::string> &vars){
	vars.insert(var);
}


BinExp::BinExp(enum BinOp op, Expression *l, Expression *r){
	this->op = op;
	this->left = l;
	this->right = r;
}

void BinExp::emit(std::map<std::string, int> &context,
				  std::vector<Instruction> &is){

	left->emit(context, is);
	right->emit(context, is);

	Instruction operation = add();
	switch(this->op){
		case ADD_OP:
			operation = add();
			break;
		case MIN_OP:
			operation = min();
			break;
		case MUL_OP:
			operation = mul();
			break;
		case DIV_OP:
			operation = div();
			break;
		case EQ_OP:
			operation = eq();
			break;
		case LT_OP:
			operation = lt();
			break;
		case LTE_OP:
			operation = lte();
			break;
		case GT_OP:
			operation = gt();
			break;
		case GTE_OP:
			operation = gte();
			break;
	}
	is.push_back(operation);
}

void BinExp::get_variables(std::set<std::string> &vars){
	left->get_variables(vars);
	right->get_variables(vars);
}

CallExp::CallExp(std::string name, std::initializer_list<Expression *> args){
	this->name = name;
	for(auto a : args){
		this->arguments.push_back(a);
	}
}

CallExp::CallExp(std::string name, std::vector<Expression *> args){
	this->name = name;
	for(auto a : args){
		this->arguments.push_back(a);
	}
}

void CallExp::emit(std::map<std::string, int> &context,
				   std::vector<Instruction> &is){
	
	for(auto a : arguments){
		a->emit(context, is);
	}
	is.push_back( push_frame(arguments.size()) );
	is.push_back( jmp_lbl( strdup(name.c_str()) ) );
}

void CallExp::get_variables(std::set<std::string> &vars){
	for(auto arg : arguments){
		arg->get_variables(vars);
	}
}

ClosureCallExp::ClosureCallExp(
	Expression *exp,
	std::initializer_list<Expression *> args
	){
	this->closure = exp;
	for(auto a : args){
		this->arguments.push_back(a);
	}
}

ClosureCallExp::ClosureCallExp(
				Expression *exp,
				std::vector<Expression *> args
				){
	this->closure = exp;
	for(auto a : args){
		this->arguments.push_back(a);
	}
}

void ClosureCallExp::emit(
			std::map<std::string, int> &context,
			std::vector<Instruction> &is
			){
	
	for(auto a : arguments){
		a->emit(context, is);
	}
	closure->emit(context, is);
	is.push_back( push_frame(arguments.size()+1) );
	is.push_back( jmp_closure() );
}

void ClosureCallExp::get_variables(std::set<std::string> &vars){
	closure->get_variables(vars);
	for(auto arg : arguments){
		arg->get_variables(vars);
	}
}

GetFieldExp::GetFieldExp(std::string field, Expression *exp){
	this->field = field;
	this->expression = exp;
}

void GetFieldExp::emit(std::map<std::string, int> &context,
					   std::vector<Instruction> &is){
	expression->emit(context, is);
	is.push_back( lookup_s( strdup(field.c_str())) );
}

ClosureExp::ClosureExp(
				std::vector<std::string> args,
				BlockStmt *body){

	for(auto arg : args){
		arguments.push_back(arg);
	}
	this->body = body;
}

void GetFieldExp::get_variables(std::set<std::string> &vars){
	expression->get_variables(vars);
}

ClosureExp::ClosureExp(
				std::initializer_list<std::string> args,
				BlockStmt *body){

	for(auto arg : args){
		arguments.push_back(arg);
	}
	this->body = body;
}


void ClosureExp::get_variables(std::set<std::string> &vars){
	body->get_variables(vars);
}

void ClosureExp::emit(std::map<std::string, int> &context,
						std::vector<Instruction> &is){
	

	/*
	 * TODO: Because we use a temporary list of instructions
	 * here, nested closure may have the wrong absolute address.
	 *
	 * Consider: why am I even using absolute addressing in the first
	 * place? Answer: closures may be called from a variety of places.
	 *
	 * The only suitable solutions is to pass the offset to emit
	 * (annoying to implement).
	 * Or to add a new Closure label instruction which is changed
	 * to absolute address during processing.
	 */
	/*
	 * Captured variables are variables that are used,
	 * but not defined in either the closure's scope or 
	 * global scope. So a variable is captured when it
	 * is used and also present in the outside context (&context).
	 *
	 * Solution store count of captured vars in closure.
	 * Then process labels uses that number to create the absolute addr.
	 */
	std::set<std::string> used;
	get_variables(used);
	int captured = 0;

	puts("captured variables:");
	for(auto s : used){
		if(context.count(s) > 0){
			std::cout << s << std::endl;
			captured++;
		}
	}
	puts("=================");
	printf("captured %d\n", captured);

	/*
	 * Set up the closure creation
	 * we compile the actual closure
	 * just after the closure create
	 *
	 * instr
	 * 		new_closure
	 * 		capture vars [0 .. n]
	 * 		jmp over closure
	 * 		closure_start (label)
	 */
	int clos_addr_abs = is.size()+3+ captured; //TODO: consider 3
	printf("captured %d\n", captured);

	//is.push_back( new_closure(clos_addr_abs) );
	is.push_back( new_closure(captured) );

	for(auto s : used){
		if(context.count(s) > 0){
			int addr = context[s];
			is.push_back( closure_capture(addr) );
		}
	}
	/*
	 * Now we need to make a new context containing the
	 * args and local variables stack locations.
	 *
	 * Captured variables are added last.
	 */

	auto func_context = std::map<std::string, int>();
	int stack_pos = 0;
	for(auto arg : arguments){
		func_context[arg] = stack_pos;
		stack_pos++;
	}

	auto locals = std::vector<std::string>();
	body->find_DeclareStmts(locals);
	for(auto var : locals){
		func_context[var] = stack_pos;
		stack_pos++;
	}

	// adding captured vars to context
	for(auto s : used){
		if(context.count(s) > 0){
			int addr = context[s];
			func_context[s] = stack_pos;
			stack_pos++;
		}
	}
	
	auto function_is = std::vector<Instruction>();
	function_is.push_back( label("anon_function") ); // push label
	//push space on stack for local variables
	for(auto _ : locals){
		function_is.push_back( new_obj() ); // push label
	}

	body->emit(func_context, function_is);

	is.push_back( jmp( function_is.size()+1 ) ); // jump over function body

	for( auto instr : function_is){
		is.push_back(instr);
	}
}


/*
 * STATEMENTS
 */

void Statement::get_variables(std::set<std::string> &vars){
	//default: does nothing.
}

ReturnStmt::ReturnStmt(Expression *exp){
	this->exp = exp;
}

void ReturnStmt::emit(std::map<std::string, int> &context,
					  std::vector<Instruction> &is){
	this->exp->emit(context, is);
	is.push_back( ret() );
}

void ReturnStmt::get_variables(std::set<std::string> &vars){
	exp->get_variables(vars);
}

DeclareStmt::DeclareStmt(Expression *exp, char *var){
	this->exp = exp;
	this->var = var;
}

bool DeclareStmt::is_DeclareStmt(void){
	return true;
}

void DeclareStmt::find_DeclareStmts(std::vector<std::string> &context){
	context.push_back(this->var);
}

void DeclareStmt::emit(std::map<std::string, int> &context,
					   std::vector<Instruction> &is){
	/*
	 * When emitting, this just turns into a AssignStmt without the opportunity
	 * for the variable to be global
	 */
	this->exp->emit(context, is);
	if(context.count(this->var) != 0){
		int &stack_pos = context.at(this->var);
		is.push_back(set_stk(stack_pos));
	}
}

void DeclareStmt::get_variables(std::set<std::string> &vars){
	exp->get_variables(vars);
}

AssignStmt::AssignStmt(Expression *exp, char *var){
	this->exp = exp;
	this->var = var;
}

void AssignStmt::emit(std::map<std::string, int> &context,
					  std::vector<Instruction> &is){
	this->exp->emit(context, is);
	if(context.count(this->var) != 0){
		int &stack_pos = context.at(this->var);
		is.push_back(set_stk(stack_pos));
	} else{
		char* s = this->var;
		is.push_back(set_glb(s));
		std::cout << "global assign\n";
	}
}

void AssignStmt::get_variables(std::set<std::string> &vars){
	exp->get_variables(vars);
}

BlockStmt::BlockStmt(std::vector<Statement *> inits){
	for(auto stmt : inits){
		this->statements.push_back(stmt);
	}
}

BlockStmt::BlockStmt(std::initializer_list<Statement *> inits){
	for(auto stmt : inits){
		this->statements.push_back(stmt);
	}
}

void BlockStmt::find_DeclareStmts(std::vector<std::string> &context){
	for(auto stmt : this->statements){
		stmt->find_DeclareStmts(context);
	}
}

void BlockStmt::emit(std::map<std::string, int> &context,
					 std::vector<Instruction> &is){
	for(auto stmt : this->statements){
		stmt->emit(context, is);
	}
}

void BlockStmt::get_variables(std::set<std::string> &vars){
	for(auto stmt : statements){
		stmt->get_variables(vars);
	}
}

IfStmt::IfStmt(Expression *exp, BlockStmt *block){
	this->exp = exp;
	this->_if = block;
	this->_else = new BlockStmt({});
}

IfStmt::IfStmt(Expression *exp, BlockStmt *block, BlockStmt *block2){
	this->exp = exp;
	this->_if = block;
	this->_else = block2;
}

void IfStmt::find_DeclareStmts(std::vector<std::string> &context){
	this->_if->find_DeclareStmts(context);
	this->_else->find_DeclareStmts(context);
}

void IfStmt::emit(std::map<std::string, int> &context,
				  std::vector<Instruction> &is){
	/*
	 * We need to know the size of the body so
	 * that we can jmp over it.
	 */
	auto if_is = std::vector<Instruction>();
	auto else_is = std::vector<Instruction>();
	this->_if->emit(context, if_is);
	this->_else->emit(context, else_is);

	int if_size = if_is.size();
	if(if_size == 0) if_size = 1;

	int else_size = else_is.size();
	//if(else_size == 0) else_size = 1;

	this->exp->emit(context, is);
	is.push_back( jmp_cnd(2) ); // jump into _if
	is.push_back( jmp(if_size+2) ); // jump past the _if into _else
	for(auto instr : if_is){
		is.push_back(instr);
	}
	is.push_back( jmp(else_size+1) ); //jump past else 
	for(auto instr : else_is){
		is.push_back(instr);
	}
}

void IfStmt::get_variables(std::set<std::string> &vars){
	exp->get_variables(vars);
	_if->get_variables(vars);
	if(_else != nullptr) _else->get_variables(vars);
}


WhileStmt::WhileStmt(Expression *exp, BlockStmt *block){
	this->exp = exp;
	this->body = block;
}

void WhileStmt::find_DeclareStmts(std::vector<std::string> &context){
	this->body->find_DeclareStmts(context);
}

void WhileStmt::emit(std::map<std::string, int> &context,
					 std::vector<Instruction> &is){
	auto cnd_is = std::vector<Instruction>();
	auto body_is = std::vector<Instruction>();

	exp->emit(context, cnd_is);
	body->emit(context, body_is);

	for(auto instr : cnd_is){
		is.push_back(instr);
	}

	is.push_back( jmp_cnd(2) );
	is.push_back( jmp(body_is.size() + 2) );

	for(auto instr : body_is){
		is.push_back(instr);
	}

	int total_size = 2 + cnd_is.size() + body_is.size();
	is.push_back( jmp(-total_size) );

}

void WhileStmt::get_variables(std::set<std::string> &vars){
	exp->get_variables(vars);
	body->get_variables(vars);
}

FunctionStmt::FunctionStmt(
				std::string name, 
				std::vector<std::string> args,
				BlockStmt *body){

	for(auto arg : args){
		arguments.push_back(arg);
	}
	this->name = name;
	this->body = body;
}

FunctionStmt::FunctionStmt(
				std::string name, 
				std::initializer_list<std::string> args,
				BlockStmt *body){

	for(auto arg : args){
		arguments.push_back(arg);
	}
	this->name = name;
	this->body = body;
}

void FunctionStmt::emit(std::map<std::string, int> &context,
						std::vector<Instruction> &is){
	
	/*
	 * First we need to make a new context containing the
	 * args and local variables stack locations.
	 */

	auto func_context = std::map<std::string, int>();
	int stack_pos = 0;
	for(auto arg : arguments){
		func_context[arg] = stack_pos;
		stack_pos++;
	}

	auto locals = std::vector<std::string>();
	body->find_DeclareStmts(locals);
	for(auto var : locals){
		func_context[var] = stack_pos;
		stack_pos++;
	}
	
	auto function_is = std::vector<Instruction>();
	function_is.push_back( label(strdup(name.c_str())) ); // push label
	//push space on stack for local variables
	for(auto _ : locals){
		function_is.push_back( new_obj() ); // push label
	}

	body->emit(func_context, function_is);

	is.push_back( jmp( function_is.size()+1 ) ); // jump over function body

	for( auto instr : function_is){
		is.push_back(instr);
	}
}

SetFieldStmt::SetFieldStmt(std::string field,
						   Expression *obj,
						   Expression *exp){
	this->field = field;
	this->object = obj;
	this->expression = exp;
}

void SetFieldStmt::emit(std::map<std::string, int> &context,
						std::vector<Instruction> &is){
	expression->emit(context, is);
	object->emit(context, is);
	is.push_back( insert_s( strdup(field.c_str())) );
}

void SetFieldStmt::get_variables(std::set<std::string> &vars){
	object->get_variables(vars);
	expression->get_variables(vars);
}
