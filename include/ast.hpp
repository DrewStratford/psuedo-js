#ifndef  AST_HPP
#define  AST_HPP

#include "parser.h"
#include "instruction.hpp"

class Expression{
	public:
	virtual void emit(std::map<std::string, int> &,std::vector<Instruction> &) = 0;
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
	private:

	public:
	ObjectExp();
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
};

class VarExp : public Expression{
	private:
	char * var;

	public:
	VarExp(char *);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
};

enum BinOp {ADD_OP, MIN_OP, MUL_OP, DIV_OP, EQ_OP, LT_OP, LTE_OP, GT_OP, GTE_OP};
class BinExp : public Expression{
	private:
	enum BinOp op;
	Expression *left;
	Expression *right;
	

	public:
	BinExp(enum BinOp, Expression *, Expression *);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
};

class CallExp : public Expression{
	private:
	std::string name;
	std::vector<Expression *> arguments;

	public:
	CallExp(std::string, std::initializer_list<Expression *>);
	CallExp(std::string, std::vector<Expression *>);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
};

class GetFieldExp : public Expression{
	private:
	std::string field;
	Expression *expression;

	public:
	GetFieldExp(std::string, Expression *);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
};


class Statement{
	public:
	virtual void emit(std::map<std::string, int> &,std::vector<Instruction> &) = 0;
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
};

class AssignStmt : public Statement{
	private:
	Expression *exp;
	char *var;

	public:
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
};

class WhileStmt : public Statement{
	private:
	Expression *exp;
	BlockStmt *body;

	public:
	WhileStmt(Expression *, BlockStmt *);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
	virtual void find_DeclareStmts(std::vector<std::string>&);
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
	std::string field;
	Expression *object;
	Expression *expression;

	public:
	SetFieldStmt(std::string, Expression *obj, Expression *exp);
	void emit(std::map<std::string, int> &, std::vector<Instruction> &);
};

#endif
