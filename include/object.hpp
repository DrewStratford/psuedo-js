#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <vector>
#include <list>
#include <map>
#include <iterator>


class Closure;

enum ObjType {INT, FLOAT, OBJECT, VECTOR, UNIT, CLOSURE};
class Object{

	private:
	/*
	 * this is stuff for the garbage collection
	 */
	static Object object_list;

	Object *prev = nullptr;
	Object *next = nullptr;
	bool active = false; //GC helper

	void insert_after(Object *o);
	void remove(void);


	public:
	enum ObjType type;
	union{
		int i;
		float f;
		std::map<std::string, Object *> *obj;
		std::vector<Object *> *vec;
		Closure *closure;
	};

	Object(); 
	Object(int i);
	Object(float f);
	Object(ObjType t);
	Object(Closure *c);
	~Object();

	bool equals(Object *o);
	enum ObjType get_type(void);
	Closure *get_closure(void);

	void capture_var(Object *o);

	void show(void);

	Object *lookup(std::string str);
	void set(std::string str, Object *o);
	void set_idx(Object *idx, Object *o);
	Object *lookup_idx(Object *idx);

	Object *size(void);

	//these should probably be private
	friend void gc_sweep_vector(std::vector<Object*> *vec);
	friend void gc_sweep_map(std::map<std::string, Object*> *map);
	static void gc_delete(void);


	/*
	 * These functions will create a lot of garbage
	 * which sucks.
	 */
	friend Object * add(Object *, Object *);
	friend Object * sub(Object *, Object *);
	friend Object * mul(Object *, Object *);
	friend Object * mod(Object *, Object *);
	friend Object * div(Object *, Object *);

	friend Object * lt(Object *, Object *);
	friend Object * lte(Object *, Object *);
	friend Object * gt(Object *, Object *);
	friend Object * gte(Object *, Object *);
};


class Closure{
	private:
	int func_ptr;
	public:
	std::vector<Object *> env;
	Closure(int f_ptr);
	int get_func(void);
	void push_var(Object *o);
	void print_env(void);
};



#endif
