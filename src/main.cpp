#include <iostream>
#include "object.hpp"
#include "context.hpp"
#include "instruction.hpp"
#include "ast.hpp"


int main(void){

	Context ctx = Context();

	std::vector<Instruction> is= std::vector<Instruction>();
	/*
	//is.push_back( push_frame());
	
	//is.push_back( jmp(18));	// jump passed factorial definition
	is.push_back( jmp(19));	// jump passed factorial definition
	//is.push_back( load_imm_i(8));	// 0 count
	is.push_back( label("factorial"));
	is.push_back( load_imm_i(1));	// 1 acc

	is.push_back( load_stk(0));
	is.push_back( load_imm_i(0));
	is.push_back( eq());
	is.push_back( jmp_cnd(11));	// jump to end of loop

	// acc *= count
	is.push_back( load_stk(0));
	is.push_back( load_stk(1));
	is.push_back( mul());
	is.push_back( set_stk(1));

	//acc --
	is.push_back( load_imm_i(1));
	is.push_back( load_stk(0));
	is.push_back( min());
	is.push_back( set_stk(0));

	is.push_back( jmp(-13));	// jump to beginning of loop

	is.push_back( show_frame());// return the acc
	is.push_back( load_stk(1));
	is.push_back( ret());

	is.push_back( show_frame());
	
	is.push_back( push_frame());	//set up call frame
	is.push_back( load_imm_i(5));	// push argument
	//is.push_back( jmp_lnk(-20));	// call factorial
	is.push_back( jmp_lbl("factorial"));	// call factorial


	is.push_back( show_frame());
	*/

	auto c = std::map<std::string, int>();
	Expression * exp = 
		new BinExp(
			MUL_OP, 
			new IntExp(2), 
			new BinExp(
				ADD_OP,
				new IntExp(3), new IntExp(6) 
				)
		);
	//Statement *stmt = 
	//	new BlockStmt({
	//		new AssignStmt(exp, "var"),
	//		new BlockStmt({ 
	//			new AssignStmt(
	//				new IntExp(200), 
	//				"var"
	//			)
	//		}),
	//		new IfStmt(
	//			new IntExp(0), //cnd

	//			new BlockStmt({  // then
	//				new AssignStmt(
	//					new BinExp(ADD_OP, new IntExp(2), new VarExp("var") ), 
	//					"other_var"
	//				)
	//			}),

	//			new BlockStmt({ // else
	//				new AssignStmt(new IntExp(1), "other_var")
	//			})
	//		)
	//	});
	
	Statement *stmt =
		new BlockStmt({
			new FunctionStmt(
				"foo",
				{ "i" },
				new BlockStmt({
					new DeclareStmt( new VarExp("i"), "c"),
					new AssignStmt(
						new BinExp(MUL_OP, new IntExp(2), new VarExp("c")),
						"c"
					),
							
					new ReturnStmt(
						new BinExp(MIN_OP, new IntExp(50), new VarExp("c"))
					)
				})
			),

			new FunctionStmt(
				"factorial",
				{ "i" },
				new BlockStmt({
					new DeclareStmt( new IntExp(1), "out"),
					new DeclareStmt( new IntExp(1), "count"),
					
					new ReturnStmt( new VarExp("out") )
				})
			),

			
			new AssignStmt( new CallExp("foo", { new IntExp(100) }), "var"),
			new AssignStmt( new ObjectExp(), "obj"),

			new SetFieldStmt("myint", new VarExp("obj"), new IntExp(123456))
		});

	stmt->emit(c, is);
	is.push_back( show_frame());


	process_labels(is);

	

	auto globals = new Object(OBJECT);
	for(int ip = 0; ip >= 0 && ip < is.size(); ){
		auto i = is[ip];
		step_instruction(&ctx, i, &ip, globals);
	}

	globals->show();
	ctx.garbage_collect();

	return 0;
}
