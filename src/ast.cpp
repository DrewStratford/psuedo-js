#include "ast.hpp"
#include <iostream>


void Expression::get_variables(std::set<std::string> &vars){
}

UnitExp::UnitExp(void){}

void UnitExp::emit(CompilationState& state, ScopeInfo &context,
				  std::vector<Instruction> &is){
	is.push_back( new_unit() );
}


IntExp::IntExp(int i){
	this->i = i;
}

void IntExp::emit(CompilationState& state, ScopeInfo &context,
				  std::vector<Instruction> &is){
	is.push_back(load_imm_i(this->i));
}

FloatExp::FloatExp(float f){
	this->f = f;
}

void FloatExp::emit(CompilationState& state, ScopeInfo &context, 
					std::vector<Instruction> &is){
	is.push_back(load_imm_f(this->f));
}

ObjectExp::ObjectExp(){
}

void ObjectExp::emit(CompilationState& state, ScopeInfo &context,
					 std::vector<Instruction> &is){
	is.push_back( new_obj() );
}

VectorExp::VectorExp(){ }

VectorExp::VectorExp(std::vector<ExprPtr > &es){
	for(auto exp : es){
		elems.push_back(exp);
	}
}

void VectorExp::emit(CompilationState& state, ScopeInfo &context,
					 std::vector<Instruction> &is){
	is.push_back( new_vec() );
	//we push the elems then add them to the newly
	// created vector
	for(auto exp : elems){
		exp->emit(state, context, is);
		is.push_back( add() );
	}
}

void VectorExp::get_variables(std::set<std::string> &vars){
	for(auto exp : elems){
		exp->get_variables(vars);
	}
}

StringExp::StringExp(std::vector<char>& cs){
	str = std::string(cs.begin(), cs.end());
}

void StringExp::emit(CompilationState& state, ScopeInfo &context,
					 std::vector<Instruction> &is){
	is.push_back( new_string(str.c_str()) );
}

void StringExp::get_variables(std::set<std::string> &vars){ }

VarExp::VarExp(const std::string& s){
	var_name = s;
}

void VarExp::emit(CompilationState& state, ScopeInfo &context,
				  std::vector<Instruction> &is){
	if(context.count(var_name) != 0){
		int &stack_pos = context.at(var_name);
		is.push_back(load_stk(stack_pos));
	} else{
		is.push_back(load_glb(var_name.c_str()));
	}
}

void VarExp::get_variables(std::set<std::string> &vars){
	vars.insert(var_name);
}


BinExp::BinExp(enum BinOp op, ExprPtr l, ExprPtr r){
	this->op = op;
	this->left = l;
	this->right = r;
}

void BinExp::emit(CompilationState& state, ScopeInfo &context,
				  std::vector<Instruction> &is){

	left->emit(state, context, is);
	right->emit(state, context, is);

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
		case MOD_OP:
			operation = mod();
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

CallExp::CallExp(std::string name, std::initializer_list<ExprPtr > args){
	this->name = name;
	for(auto a : args){
		this->arguments.push_back(a);
	}
}

CallExp::CallExp(std::string name, std::vector<ExprPtr > args){
	this->name = name;
	for(auto a : args){
		this->arguments.push_back(a);
	}
}

void CallExp::emit(CompilationState& state, ScopeInfo &context,
				   std::vector<Instruction> &is){
	
	for(auto a : arguments){
		a->emit(state, context, is);
	}
	is.push_back( push_frame(arguments.size()) );
	is.push_back( jmp_lbl(name.c_str()) );
}

void CallExp::get_variables(std::set<std::string> &vars){
	for(auto arg : arguments){
		arg->get_variables(vars);
	}
}

ClosureCallExp::ClosureCallExp(
	ExprPtr exp,
	std::initializer_list<ExprPtr > args
	){
	this->is_setter = false;
	this->closure = exp;
	for(auto a : args){
		this->arguments.push_back(a);
	}
}

ClosureCallExp::ClosureCallExp(
				ExprPtr exp,
				std::vector<ExprPtr > args
				){
	for(auto a : args){
		this->arguments.push_back(a);
	}
	this->closure = exp;
}

void ClosureCallExp::emit(CompilationState& state, 
			ScopeInfo &context,
			std::vector<Instruction> &is
			){
	
	for(auto a : arguments){
		a->emit(state, context, is);
	}
	closure->emit(state, context, is);
	is.push_back( push_frame(arguments.size()+1) );
	is.push_back( jmp_closure() );
}

void ClosureCallExp::get_variables(std::set<std::string> &vars){
	for(auto arg : arguments){
		arg->get_variables(vars);
	}
	closure->get_variables(vars);
}

void ClosureCallExp::set_setter(bool b){ }
void ClosureCallExp::set_sub_expr(ExprPtr exp){
	closure = exp;
}

FFICallExp::FFICallExp(
				std::string func,
				std::vector<ExprPtr > args
				){
	this->func_name = func;
	for(auto a : args){
		this->arguments.push_back(a);
	}
}

void FFICallExp::emit(CompilationState& state, 
			ScopeInfo &context,
			std::vector<Instruction> &is
			){
	
	for(auto a : arguments){
		a->emit(state, context, is);
	}
	is.push_back( push_frame(arguments.size()) );
	is.push_back( ffi_call_sym(func_name.c_str()) );
}

void FFICallExp::get_variables(std::set<std::string> &vars){
	for(auto arg : arguments){
		arg->get_variables(vars);
	}
}

void AccessorExp::set_setter(bool flag){
	is_setter = flag;
}


FieldAccessor::FieldAccessor(std::string f){
	field = f;
}
void FieldAccessor::emit(CompilationState& state, ScopeInfo &context,
					   std::vector<Instruction> &is){
	sub_exp->emit(state, context, is);
	if(is_setter){
		is.push_back( insert_s(field.c_str()) );
	} else{
		is.push_back( lookup_s(field.c_str()) );
	}
}

void FieldAccessor::set_sub_expr(ExprPtr exp){
	sub_exp = exp;
}

void FieldAccessor::get_variables(std::set<std::string> &vars){
	sub_exp->get_variables(vars);
}

ArrayAccessor::ArrayAccessor(ExprPtr exp){
	this->exp = exp;
}

void ArrayAccessor::emit(CompilationState& state, ScopeInfo &context,
					   std::vector<Instruction> &is){
	sub_exp->emit(state, context, is);
	exp->emit(state, context, is);
	if(is_setter){
		is.push_back( insert_v() );
	} else{
		is.push_back( lookup_v() );
	}
}

void ArrayAccessor::get_variables(std::set<std::string> &vars){
	sub_exp->get_variables(vars);
	exp->get_variables(vars);
}

void ArrayAccessor::set_sub_expr(ExprPtr exp){
	sub_exp = exp;
}

/*
 * This now does the double duty of "getting" from arrays and
 * objects.
 */
GetFieldExp::GetFieldExp(AccessPtr acc, ExprPtr exp){
	acc->set_setter(false);
	this->accessor = acc;
	this->expression = exp;
}

void GetFieldExp::emit(CompilationState& state, ScopeInfo &context,
					   std::vector<Instruction> &is){
	expression->emit(state, context, is);
	accessor->emit(state, context, is);
}

void GetFieldExp::get_variables(std::set<std::string> &vars){
	expression->get_variables(vars);
	accessor->get_variables(vars);
}

ClosureExp::ClosureExp(
				std::vector<std::string> args,
				std::shared_ptr<BlockStmt> body){

	for(auto arg : args){
		arguments.push_back(arg);
	}
	this->body = body;
}

ClosureExp::ClosureExp(
				std::initializer_list<std::string> args,
				std::shared_ptr<BlockStmt> body){

	for(auto arg : args){
		arguments.push_back(arg);
	}
	this->body = body;
}


void ClosureExp::get_variables(std::set<std::string> &vars){
	body->get_variables(vars);
}

void ClosureExp::emit(CompilationState& state, ScopeInfo &context,
						std::vector<Instruction> &is){
	std::set<std::string> used;
	get_variables(used);
	int captured = 0;

	for(auto s : used){
		if(context.count(s) > 0){
			std::cout << s << std::endl;
			captured++;
		}
	}

	/*
	 * Set up the closure creation
	 * we compile the actual closure
	 * just after the closure create
	 *
	 * instr
	 * 		new_closure
	 * 		capture vars
	 * 		jmp over closure
	 * 		closure_start (label)
	 *
	 * 	Once compiled, process labels will calculate the absolute address using the
	 * 	captuer count.
	 */
	is.push_back( new_closure(captured) );

	for(auto s : used){
		if(context.count(s) == 0) continue;

		int addr = context[s];
		is.push_back( closure_capture(addr) );
	}

	/*
	 * Now we need to make a new context containing the
	 * args and local variables stack locations.
	 *
	 * Captured variables are added last.
	 */
	auto func_context = ScopeInfo();
	int stack_pos = 0;
	for(auto arg : arguments){
		func_context[arg] = stack_pos;
		stack_pos++;
	}

	// adding captured vars to context
	for(auto s : used){
		if(context.count(s) == 0) continue;

		int addr = context[s];
		func_context[s] = stack_pos;
		stack_pos++;
	}

	auto locals = std::vector<std::string>();
	body->find_DeclareStmts(locals);
	for(auto var : locals){
		func_context[var] = stack_pos;
		stack_pos++;
	}

	auto function_is = std::vector<Instruction>();
	function_is.push_back( label("anon_function") ); // push label
	//push space on stack for local variables
	for(auto _ : locals){
		function_is.push_back( new_obj() ); // push label
	}

	body->emit(state, func_context, function_is);

	is.push_back( jmp( function_is.size()+1 ) ); // jump over function body

	is.insert(is.begin(), function_is.begin(), function_is.end());
}


/*
 * STATEMENTS
 */

void Statement::get_variables(std::set<std::string> &vars){
	//default: does nothing.
}

ExpressionStmt::ExpressionStmt(ExprPtr exp){
	this->exp = exp;
}

void ExpressionStmt::emit(CompilationState& state, ScopeInfo &context,
					  std::vector<Instruction> &is){
	this->exp->emit(state, context, is);
	is.push_back( drop() );
}

void ExpressionStmt::get_variables(std::set<std::string> &vars){
	exp->get_variables(vars);
}

ReturnStmt::ReturnStmt(ExprPtr exp){
	this->exp = exp;
}

void ReturnStmt::emit(CompilationState& state, ScopeInfo &context,
					  std::vector<Instruction> &is){
	this->exp->emit(state, context, is);
	is.push_back( ret() );
}

void ReturnStmt::get_variables(std::set<std::string> &vars){
	exp->get_variables(vars);
}

DeclareStmt::DeclareStmt(ExprPtr exp, const std::string& var){
	this->exp = exp;
	this->var_name = var;
}

bool DeclareStmt::is_DeclareStmt(void){
	return true;
}

void DeclareStmt::find_DeclareStmts(std::vector<std::string> &context){
	context.push_back(var_name);
}

void DeclareStmt::emit(CompilationState& state, ScopeInfo &context,
					   std::vector<Instruction> &is){
	/*
	 * When emitting, this just turns into a AssignStmt without the opportunity
	 * for the variable to be global
	 */
	this->exp->emit(state, context, is);
	if(context.count(var_name) != 0){
		int &stack_pos = context.at(var_name);
		is.push_back(set_stk(stack_pos));
	}
}

void DeclareStmt::get_variables(std::set<std::string> &vars){
	exp->get_variables(vars);
}

AssignStmt::AssignStmt(ExprPtr exp, const std::string& var){
	this->exp = exp;
	this->var = var;
}

void AssignStmt::emit(CompilationState& state, ScopeInfo &context,
					  std::vector<Instruction> &is){
	this->exp->emit(state, context, is);
	if(context.count(this->var) != 0){
		int &stack_pos = context.at(this->var);
		is.push_back(set_stk(stack_pos));
	} else{
		is.push_back(set_glb(this->var.c_str()));
	}
}

void AssignStmt::get_variables(std::set<std::string> &vars){
	exp->get_variables(vars);
}

BlockStmt::BlockStmt(std::vector<StmtPtr> inits){
	for(auto stmt : inits){
		this->statements.push_back(stmt);
	}
}

BlockStmt::BlockStmt(std::initializer_list<StmtPtr> inits){
	for(auto stmt : inits){
		this->statements.push_back(stmt);
	}
}

void BlockStmt::find_DeclareStmts(std::vector<std::string> &context){
	for(auto stmt : this->statements){
		stmt->find_DeclareStmts(context);
	}
}

void BlockStmt::emit(CompilationState& state, ScopeInfo &context,
					 std::vector<Instruction> &is){
	for(auto stmt : this->statements){
		stmt->emit(state, context, is);
	}
}

void BlockStmt::get_variables(std::set<std::string> &vars){
	for(auto stmt : statements){
		stmt->get_variables(vars);
	}
}

IfStmt::IfStmt(ExprPtr exp, StmtPtr block){
	this->exp = exp;
	this->_if = block;
	this->_else = std::shared_ptr<BlockStmt>(new BlockStmt({}));
}

IfStmt::IfStmt(ExprPtr exp, StmtPtr block, StmtPtr block2){
	this->exp = exp;
	this->_if = block;
	this->_else = block2;
}

void IfStmt::find_DeclareStmts(std::vector<std::string> &context){
	this->_if->find_DeclareStmts(context);
	this->_else->find_DeclareStmts(context);
}

void IfStmt::emit(CompilationState& state, ScopeInfo &context,
				  std::vector<Instruction> &is){
	 // We need to know the size of the body so
	 // that we can jmp over it.
	auto if_is = std::vector<Instruction>();
	auto else_is = std::vector<Instruction>();
	this->_if->emit(state, context, if_is);
	this->_else->emit(state, context, else_is);

	int if_size = if_is.size();
	if(if_size == 0) if_size = 1;

	int else_size = else_is.size();

	this->exp->emit(state, context, is);
	is.push_back( jmp_cnd(2) ); // jump into _if
	is.push_back( jmp(if_size+2) ); // jump past the _if into _else
	is.insert(is.begin(), if_is.begin(), if_is.end());
	is.push_back( jmp(else_size+1) ); //jump past else 
	is.insert(is.begin(), else_is.begin(), else_is.end());
}

void IfStmt::get_variables(std::set<std::string> &vars){
	exp->get_variables(vars);
	_if->get_variables(vars);
	if(_else != nullptr) _else->get_variables(vars);
}


WhileStmt::WhileStmt(ExprPtr exp, std::shared_ptr<BlockStmt> block){
	this->exp = exp;
	this->body = block;
}

void WhileStmt::find_DeclareStmts(std::vector<std::string> &context){
	this->body->find_DeclareStmts(context);
}

void WhileStmt::emit(CompilationState& state, ScopeInfo &context,
					 std::vector<Instruction> &is){
	auto cnd_is = std::vector<Instruction>();
	auto body_is = std::vector<Instruction>();

	exp->emit(state, context, cnd_is);
	body->emit(state, context, body_is);

	is.insert(is.begin(), cnd_is.begin(), cnd_is.end());

	is.push_back( jmp_cnd(2) );
	is.push_back( jmp(body_is.size() + 2) );

	is.insert(is.begin(), body_is.begin(), body_is.end());

	int total_size = 2 + cnd_is.size() + body_is.size();
	is.push_back( jmp(-total_size) );

}

void WhileStmt::get_variables(std::set<std::string> &vars){
	exp->get_variables(vars);
	body->get_variables(vars);
}

FunctionStmt::FunctionStmt(
				const std::string& name, 
				std::vector<std::string> args,
				std::shared_ptr<BlockStmt> body){

	for(auto arg : args){
		arguments.push_back(arg);
	}
	this->name = name;
	this->body = body;
}

FunctionStmt::FunctionStmt(
				const std::string& name, 
				std::initializer_list<std::string> args,
				std::shared_ptr<BlockStmt> body){

	for(auto arg : args){
		arguments.push_back(arg);
	}
	this->name = name;
	this->body = body;
}

void FunctionStmt::emit(CompilationState& state, ScopeInfo &context,
						std::vector<Instruction> &is){
	
	/*
	 * First we need to make a new context containing the
	 * args and local variables stack locations.
	 */

	auto func_context = ScopeInfo();
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
	function_is.push_back( label(name.c_str()) ); // push label
	//push space on stack for local variables
	for(auto _ : locals){
		function_is.push_back( new_obj() ); // push label
	}

	body->emit(state, func_context, function_is);

	is.push_back( jmp( function_is.size()+1 ) ); // jump over function body

	for( auto instr : function_is){
		is.push_back(instr);
	}
}

SetFieldStmt::SetFieldStmt(AccessPtr obj,
						   ExprPtr exp){
	obj->set_setter(true);
	this->object = obj;
	this->expression = exp;
}

void SetFieldStmt::emit(CompilationState& state, ScopeInfo &context,
						std::vector<Instruction> &is){
	expression->emit(state, context, is);
	object->emit(state, context, is);
}

void SetFieldStmt::get_variables(std::set<std::string> &vars){
	object->get_variables(vars);
	expression->get_variables(vars);
}

LoadFFIStmt::LoadFFIStmt(std::string name){
	this->ffi_name = name;
}

void LoadFFIStmt::emit(CompilationState& state, ScopeInfo &context,
						std::vector<Instruction> &is){
	is.push_back( ffi_load(ffi_name.c_str()) );
}
