#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <vector>
#include <list>
#include <map>
#include <iterator>
#include <string>


class Closure;
class Object;
struct object_ptr;
typedef struct object_ptr ObjPtr;

// we have 4 bits to store this in so we can have
// 8 fundamental types.
enum PointerType {INT, FLOAT, CLOSURE, ARRAY, DICT, STRING} ;

/*
 * Base class for all heap allocated data structures.
 * Mostly handles all the garbage collection stuff.
 */
class Object{

	private:
	/*
	 * this is stuff for the garbage collection
	 */
	static Object object_list;

	Object *prev = nullptr;
	Object *next = nullptr;

	void insert_after(Object *o);
	void remove(void);
	protected:
	bool active = false; //GC helper


	public:

	Object(); 

	virtual void mark(void);
	virtual std::ostream& show(std::ostream&) const;

	friend std::ostream& operator<<(std::ostream&, const Object&);

	//these should probably be private
	static void gc_delete(void);
};

void gc_sweep_vector(std::vector<ObjPtr> &vec);
void gc_sweep_map(std::map<std::string, ObjPtr> &map);

class Closure : public Object{
	private:
	public:
	int func_ptr;
	std::vector<ObjPtr> env;
	Closure(int f_ptr);
	int get_func(void);
	void push_var(ObjPtr);
	std::ostream& print_env(std::ostream&) const;

	void mark(void);
	std::ostream& show(std::ostream&) const;
};

/*
 * Here we be a bit lazy, and extend the std::vector
 * and map classes to work with our garbage collection.
 *
 * This allows us to reuse iterators etc.
 */
template<typename a>
class ArrayList_ : public Object, public std::vector<a>{
	private:
	public:

	void mark(void);
	std::ostream& show(std::ostream&) const;
};
typedef ArrayList_<ObjPtr> ArrayList;

template<typename k, typename v>
class Dictionary_ : public Object, public std::map<k, v>{
	private:
	public:

	void mark(void);
	std::ostream& show(std::ostream&) const;
};
typedef Dictionary_<std::string, ObjPtr> Dictionary;

class StringObject : public Object {
	private:
	std::string str;
	public:
	StringObject(std::string&&);
	void mark(void);
	std::ostream& show(std::ostream&) const;
	StringObject* add(StringObject* object);
};

/*
 * A 64 bit wide struct that is either
 * a pointer to an Object or a primitive
 * value like a 32 bit Integer.
 */
struct object_ptr{
	uint64_t data : 60;
	enum PointerType type : 4;

	object_ptr(void); 
	object_ptr(int32_t i);
	object_ptr(float f);
	object_ptr(Closure*);
	object_ptr(Dictionary*);
	object_ptr(ArrayList*);
	object_ptr(StringObject*);

	int32_t as_i(void);
	float as_f(void);
	Object* as_o(void);
	Closure* as_c(void);
	Dictionary* as_dict(void);
	ArrayList* as_arr(void);
	StringObject* as_string(void);

	std::ostream& show(std::ostream&);

} __attribute__((packed));

ObjPtr add(ObjPtr, ObjPtr);
ObjPtr sub(ObjPtr, ObjPtr);
ObjPtr mul(ObjPtr, ObjPtr);
ObjPtr mod(ObjPtr, ObjPtr);
ObjPtr div(ObjPtr, ObjPtr);

ObjPtr lt(ObjPtr, ObjPtr);
ObjPtr lte(ObjPtr, ObjPtr);
ObjPtr gt(ObjPtr, ObjPtr);
ObjPtr gte(ObjPtr, ObjPtr);


#endif
