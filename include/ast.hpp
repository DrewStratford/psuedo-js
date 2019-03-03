#ifndef  AST_HPP
#define  AST_HPP

#include <set>

#include "parser.h"
#include "instruction.hpp"

class Expression{
	public:
	virtual void 
	emit(std::map<std::string, int> &,std::vector<Instruction> &) = 0;

	virtual void
	get_variables(std::set<std::string> &vars);
};

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
	std::vector<Expression *> elems;

	public:
	VectorExp();
	VectorExp(std::vector<Expression *> &);
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
	BlockStmt *body;

	public:
	ClosureExp(std::initializer_list<std::string>, BlockStmt *);
	ClosureExp(std::vector<std::string>, BlockStmt *);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	void get_variables(std::set<std::string> &vars);
};

enum BinOp 
	{ADD_OP, MIN_OP, MUL_OP, DIV_OP, EQ_OP, LT_OP, LTE_OP, GT_OP, GTE_OP};
class BinExp : public Expression{
	private:
	enum BinOp op;
	Expression *left;
	Expression *right;
	

	public:
	BinExp(enum BinOp, Expression *, Expression *);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	void get_variables(std::set<std::string> &vars);
};

class CallExp : public Expression{
	private:
	std::string name;
	std::vector<Expression *> arguments;

	public:
	CallExp(std::string, std::initializer_list<Expression *>);
	CallExp(std::string, std::vector<Expression *>);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	void get_variables(std::set<std::string> &vars);
};

class ClosureCallExp : public Expression{
	private:
	Expression *closure;
	std::vector<Expression *> arguments;

	public:
	ClosureCallExp(Expression *exp, std::initializer_list<Expression *>);
	ClosureCallExp(Expression *exp, std::vector<Expression *>);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	void get_variables(std::set<std::string> &vars);
};

class FFICallExp : public Expression{
	private:
	std::string func_name;
	std::vector<Expression *> arguments;

	public:
	FFICallExp(std::string, std::vector<Expression *>);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	void get_variables(std::set<std::string> &vars);
};

class AccessorExp : public Expression{
	protected:
	bool is_setter = false;

	public:
	void set_setter(bool);
};

class FieldAccessor: public AccessorExp{
	private:
	std::string field;

	public:
	FieldAccessor(std::string);
	//void get_variables(std::set<std::string> &vars);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
};

class ArrayAccessor: public AccessorExp{
	private:
	Expression *exp;

	public:
	ArrayAccessor(Expression *exp);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	void get_variables(std::set<std::string> &vars);
};

class GetFieldExp : public Expression{
	private:
	//std::string field;
	AccessorExp *accessor;
	Expression *expression;

	public:
	//GetFieldExp(std::string, Expression *);
	GetFieldExp(AccessorExp * acc, Expression *);
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

class ReturnStmt : public Statement{
	private:
	Expression *exp;
	public:
	ReturnStmt(Expression *);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	void get_variables(std::set<std::string> &vars);
};

class DeclareStmt : public Statement{
	private:
	Expression *exp;
	char *var;

	public:
	DeclareStmt(Expression *, char *);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	virtual bool is_DeclareStmt(void);
	virtual void find_DeclareStmts(std::vector<std::string>&);
	void get_variables(std::set<std::string> &vars);
};

class AssignStmt : public Statement{
	private:
	Expression *exp;
	char *var;

	public:
	void get_variables(std::set<std::string> &vars);
	AssignStmt(Expression *, char *);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
};

class BlockStmt : public Statement{
	private:
	std::vector<Statement *> statements;

	public:
	BlockStmt(std::initializer_list<Statement *>);
	BlockStmt(std::vector<Statement *>);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	virtual void find_DeclareStmts(std::vector<std::string>&);
	void get_variables(std::set<std::string> &vars);
};

class IfStmt : public Statement{
	private:
	Expression *exp;
	BlockStmt *_if;
	BlockStmt *_else;

	public:
	IfStmt(Expression *, BlockStmt *);
	IfStmt(Expression *, BlockStmt *, BlockStmt *);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	virtual void find_DeclareStmts(std::vector<std::string>&);
	void get_variables(std::set<std::string> &vars);
};

class WhileStmt : public Statement{
	private:
	Expression *exp;
	BlockStmt *body;

	public:
	WhileStmt(Expression *, BlockStmt *);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	virtual void find_DeclareStmts(std::vector<std::string>&);
	void get_variables(std::set<std::string> &vars);
};

class FunctionStmt : public Statement{
	private:
	std::string name;
	std::vector<std::string> arguments;
	BlockStmt *body;

	public:
	FunctionStmt(std::string, std::initializer_list<std::string>, BlockStmt *);
	FunctionStmt(std::string, std::vector<std::string>, BlockStmt *);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
};

class SetFieldStmt : public Statement{
	private:
	//std::string field;
	AccessorExp *accessor;
	Expression *object;
	Expression *expression;

	public:
	//SetFieldStmt(std::string, Expression *obj, Expression *exp);
	SetFieldStmt(AccessorExp *acc, Expression *obj, Expression *exp);
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
