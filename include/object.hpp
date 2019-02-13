#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <vector>
#include <list>
#include <map>
#include <iterator>


enum ObjType {INT, FLOAT, OBJECT, VECTOR, UNIT};
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



	/*
	 *
	 */

	enum ObjType type;
	union{
		int i;
		float f;
		std::map<std::string, Object *> *obj;
		std::vector<Object *> *vec;
	};

	public:
	Object(); 
	Object(int i);
	Object(float f);
	Object(ObjType t);
	~Object();

	bool equals(Object *o);

	void show(void);

	Object *lookup(std::string str);
	void set(std::string str, Object *o);

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
	friend Object * div(Object *, Object *);

	friend Object * lt(Object *, Object *);
	friend Object * lte(Object *, Object *);
	friend Object * gt(Object *, Object *);
	friend Object * gte(Object *, Object *);

};



#endif
