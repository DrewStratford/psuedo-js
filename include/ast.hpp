#ifndef  AST_HPP
#define  AST_HPP

#include <set>
#include <memory>

#include "parser.h"
#include "instruction.hpp"

class Expression{
	public:
	virtual void 
	emit(std::map<std::string, int> &,std::vector<Instruction> &) = 0;

	virtual void
	get_variables(std::set<std::string> &vars);
};

using ExprPtr = std::shared_ptr<Expression>;

class UnitExp : public Expression{
	public:
	UnitExp(void);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
};

class IntExp : public Expression{
	private:
	int i = 0;

	public:
	IntExp(int);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
};

class FloatExp : public Expression{
	private:
	float f = 0;

	public:
	FloatExp(float);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
};

class ObjectExp : public Expression{
	public:
	ObjectExp();
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
};

class VectorExp : public Expression{
	std::vector<ExprPtr> elems;

	public:
	VectorExp();
	VectorExp(std::vector<ExprPtr> &);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	void get_variables(std::set<std::string> &vars);
};


class VarExp : public Expression{
	private:
	char * var;

	public:
	VarExp(char *);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	void get_variables(std::set<std::string> &vars);
};

class BlockStmt;
class ClosureExp : public Expression{
	private:
	std::vector<std::string> arguments;
	std::shared_ptr<BlockStmt> body;

	public:
	ClosureExp(std::initializer_list<std::string>, std::shared_ptr<BlockStmt>);
	ClosureExp(std::vector<std::string>, std::shared_ptr<BlockStmt>);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	void get_variables(std::set<std::string> &vars);
};

enum BinOp 
	{ADD_OP, MIN_OP, MUL_OP, DIV_OP, MOD_OP, EQ_OP, LT_OP,
	LTE_OP, GT_OP, GTE_OP};
class BinExp : public Expression{
	private:
	enum BinOp op;
	ExprPtr left;
	ExprPtr right;
	

	public:
	BinExp(enum BinOp, ExprPtr, ExprPtr);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	void get_variables(std::set<std::string> &vars);
};

class CallExp : public Expression{
	private:
	std::string name;
	std::vector<ExprPtr> arguments;

	public:
	CallExp(std::string, std::initializer_list<ExprPtr>);
	CallExp(std::string, std::vector<ExprPtr>);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	void get_variables(std::set<std::string> &vars);
};


class FFICallExp : public Expression{
	private:
	std::string func_name;
	std::vector<ExprPtr> arguments;

	public:
	FFICallExp(std::string, std::vector<ExprPtr >);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	void get_variables(std::set<std::string> &vars);
};

class AccessorExp : public Expression{
	protected:
	bool is_setter = false;

	public:
	virtual void set_sub_expr(ExprPtr) = 0;
	void set_setter(bool);
};

using AccessPtr = std::shared_ptr<AccessorExp>;

class FieldAccessor: public AccessorExp{
	private:
	std::string field;
	ExprPtr sub_exp;
	void get_variables(std::set<std::string> &vars);

	public:
	FieldAccessor(std::string);
	//void get_variables(std::set<std::string> &vars);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	void set_sub_expr(ExprPtr);
};

class ArrayAccessor: public AccessorExp{
	private:
	ExprPtr exp;
	ExprPtr sub_exp;

	public:
	ArrayAccessor(ExprPtr exp);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	void get_variables(std::set<std::string> &vars);
	void set_sub_expr(ExprPtr);
};

class ClosureCallExp : public AccessorExp{
	private:
	ExprPtr closure;
	std::vector<ExprPtr> arguments;

	public:
	ClosureCallExp(ExprPtr exp, std::initializer_list<ExprPtr >);
	ClosureCallExp(ExprPtr exp, std::vector<ExprPtr >);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	void get_variables(std::set<std::string> &vars);
	void set_setter(bool);
	void set_sub_expr(ExprPtr);
};

class GetFieldExp : public Expression{
	private:
	AccessPtr accessor;
	ExprPtr expression;

	public:
	GetFieldExp(AccessPtr  acc, ExprPtr );
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	void get_variables(std::set<std::string> &vars);
};


class Statement{
	public:
	virtual
	void emit(std::map<std::string, int> &,std::vector<Instruction> &) = 0;
	virtual void get_variables(std::set<std::string> &vars);
	virtual void find_DeclareStmts(std::vector<std::string>&){
	}
	virtual bool is_DeclareStmt(void){
		return false;
	}
};
using StmtPtr = std::shared_ptr<Statement>;

class ReturnStmt : public Statement{
	private:
	ExprPtr exp;
	public:
	ReturnStmt(ExprPtr );
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	void get_variables(std::set<std::string> &vars);
};

class DeclareStmt : public Statement{
	private:
	ExprPtr exp;
	char *var;

	public:
	DeclareStmt(ExprPtr , char *);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	virtual bool is_DeclareStmt(void);
	virtual void find_DeclareStmts(std::vector<std::string>&);
	void get_variables(std::set<std::string> &vars);
};

class AssignStmt : public Statement{
	private:
	ExprPtr exp;
	char *var;

	public:
	void get_variables(std::set<std::string> &vars);
	AssignStmt(ExprPtr , char *);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
};

class BlockStmt : public Statement{
	private:
	std::vector<StmtPtr> statements;

	public:
	BlockStmt(std::initializer_list<StmtPtr>);
	BlockStmt(std::vector<StmtPtr>);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	virtual void find_DeclareStmts(std::vector<std::string>&);
	void get_variables(std::set<std::string> &vars);
};

class IfStmt : public Statement{
	private:
	ExprPtr exp;
	StmtPtr _if;
	StmtPtr _else;

	public:
	IfStmt(ExprPtr , StmtPtr);
	IfStmt(ExprPtr , StmtPtr, StmtPtr);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	virtual void find_DeclareStmts(std::vector<std::string>&);
	void get_variables(std::set<std::string> &vars);
};

class WhileStmt : public Statement{
	private:
	ExprPtr exp;
	std::shared_ptr<BlockStmt> body;

	public:
	WhileStmt(ExprPtr , std::shared_ptr<BlockStmt>);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	virtual void find_DeclareStmts(std::vector<std::string>&);
	void get_variables(std::set<std::string> &vars);
};

class FunctionStmt : public Statement{
	private:
	std::string name;
	std::vector<std::string> arguments;
	std::shared_ptr<BlockStmt>body;

	public:
	FunctionStmt(std::string, std::initializer_list<std::string>, 
				std::shared_ptr<BlockStmt>);
	FunctionStmt(std::string, std::vector<std::string>, 
				std::shared_ptr<BlockStmt>);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
};

class SetFieldStmt : public Statement{
	private:
	AccessPtr object;
	ExprPtr expression;

	public:
	//SetFieldStmt(std::string, ExprPtr obj, ExprPtr exp);
	SetFieldStmt(AccessPtr obj, ExprPtr exp);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	void get_variables(std::set<std::string> &vars);
};

class LoadFFIStmt : public Statement{
	private:
	std::string ffi_name;

	public:
	LoadFFIStmt(std::string);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
};

#endif
