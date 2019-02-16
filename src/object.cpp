#include "object.hpp"

#include <iostream>

Object Object::object_list = Object();

Object::Object(){
	this->type = UNIT;
}

Object::Object(ObjType t){
	this->type = t;
	this->i = 0;

	if(t == VECTOR){
		this->vec = new std::vector<Object*>();
	} else if(t == OBJECT){
		this->obj = new std::map<std::string, Object*>();
	}

	Object::object_list.insert_after(this);
}

Object::Object(int i){
	this->type = INT;
	this->i = i;

	Object::object_list.insert_after(this);
}

Object::Object(float f){
	this->type = FLOAT;
	this->f = f;

	Object::object_list.insert_after(this);
}

Object::Object(Closure *c){
	type = CLOSURE;
	closure = c;

	Object::object_list.insert_after(this);
}

Object::~Object(){
	if(!this->active){
		//std::cout << "deleting: ";
		//this->show();
		switch(this->type){
			case OBJECT:
				delete this->obj;
				break;
			case VECTOR:
				delete this->vec;
				break;
			case CLOSURE:
				delete this->closure;
				break;
		}
	}
}

enum ObjType Object::get_type(void){
	return type;
}

Closure *Object::get_closure(void){
	return closure;
}

bool Object::equals(Object *o){
	if(o == nullptr || this->type != o->type) return false;

	bool out = false;
	switch(this->type){
		case OBJECT: out = this->obj == o->obj; break;
		case VECTOR: out = this->vec == o->vec; break;
		case INT: out = this->i == o->i; break;
		case FLOAT: out = this->f == o->f; break;
	}
	return out;	
}

/*
 * Garbage collection stuff
 */


void gc_sweep_vector(std::vector<Object*> *v){
	std::vector<Object*> &vec = *v; //need to think about how efficient this is
	for(Object * o : vec){
		if(o == nullptr || o->active) continue;
		o->active = true;
		switch(o->type){
			case VECTOR:
				gc_sweep_vector(o->vec);
				break;
			case OBJECT:
				std::cout << "OBJECT SWEEP\n";
				gc_sweep_map(o->obj);
				break;
		}
	}
}

void gc_sweep_map(std::map<std::string, Object*> *m){
	auto& map = *m;
	for(auto p : map){
		Object * o = p.second;
		if(o == nullptr || o->active) continue;
		o->active = true;
		switch(o->type){
			case VECTOR:
				gc_sweep_vector(o->vec);
				break;
			case OBJECT:
				gc_sweep_map(o->obj);
				break;
		}
	}
}

void Object::gc_delete(void){
	int freed = 0;
	int active = 0;
	Object *o = Object::object_list.next;

	while(o != nullptr){
		Object *next = o->next;

		if(o->active){
			o->active = false;
			active++;
		} else{
			o->remove();
			delete o;
			freed++;
		}

		o = next;
	}

	std::cout << "gc_delete: freed " << freed << ", active " << active << std::endl;
}
//////////////////////////////////////////////////

/*
 * list operations
 */

void Object::insert_after(Object *o){
	Object *tmp = this->next;
	this->next = o;
	o->prev = this;
	o->next = tmp;
	if(tmp != nullptr) tmp->prev = o;
}

void Object::remove(void){
	Object *tmp = this->prev;
	if(this->next != nullptr){
		this->next->prev = tmp;
	}
	tmp->next = this->next;

	this->prev = nullptr;
	this->next = nullptr;
}

//////////////////////////////////////////////////

void Object::show(void){
	switch(this->type){
		case INT:
			std::cout << this->i << std::endl;
			break;
		case FLOAT:
			std::cout << this->f << std::endl;
			break;
		case OBJECT:
			{
				std::cout << "OBJECT ..." << std::endl;
				auto& map = *this->obj;
				for(auto pair : map){
					std::cout << pair.first << " -> ";
					pair.second->show();
				}
			}
			break;
		case VECTOR:
			std::cout << "VECTOR ..." << std::endl;
			break;
		case UNIT:
			std::cout << "UNIT" << std::endl;
			break;
		case CLOSURE:
			std::cout << "CLOSURE" << std::endl;
			break;
	}
}

void Object::set(std::string str, Object *o){
	if(this->type != OBJECT) return ;
	auto &m = *this->obj;
	m[str] = o;
}

Object *Object::lookup(std::string str){
	if(this->type != OBJECT) return nullptr;
	return this->obj->at(str);
}

Object * add(Object *a, Object *b){
	if(a->type == INT && b->type == INT) return new Object(a->i+ b->i);
	if(a->type == FLOAT && b->type == FLOAT) return new Object(a->f+ b->f);
	return new Object();
}

Object * sub(Object *a, Object *b){
	if(a->type == INT && b->type == INT) return new Object(a->i- b->i);
	if(a->type == FLOAT && b->type == FLOAT) return new Object(a->f- b->f);
	return new Object();
}

Object * mul(Object *a, Object *b){
	if(a->type == INT && b->type == INT) return new Object(a->i* b->i);
	if(a->type == FLOAT && b->type == FLOAT) return new Object(a->f* b->f);
	return new Object();
}

Object * div(Object *a, Object *b){
	if(a->type == INT && b->type == INT) return new Object(a->i/ b->i);
	if(a->type == FLOAT && b->type == FLOAT) return new Object(a->f/ b->f);
	return new Object();
}

Object * lt(Object *a, Object *b){
	if(a->type == INT && b->type == INT) return new Object(a->i < b->i);
	if(a->type == FLOAT && b->type == FLOAT) return new Object(a->f < b->f);
	return new Object();
}

Object * lte(Object *a, Object *b){
	if(a->type == INT && b->type == INT) return new Object(a->i <= b->i);
	if(a->type == FLOAT && b->type == FLOAT) return new Object(a->f <= b->f);
	return new Object();
}

Object * gt(Object *a, Object *b){
	if(a->type == INT && b->type == INT) return new Object(a->i > b->i);
	if(a->type == FLOAT && b->type == FLOAT) return new Object(a->f > b->f);
	return new Object();
}

Object * gte(Object *a, Object *b){
	if(a->type == INT && b->type == INT) return new Object(a->i >= b->i);
	if(a->type == FLOAT && b->type == FLOAT) return new Object(a->f >= b->f);
	return new Object();
}


/////////////////////////////////////////////////////
// Closure stuff.
/////////////////////////////////////////////////////

Closure::Closure(int f_ptr){
	this->func_ptr = f_ptr;
}

Closure::Closure(int f_ptr, std::vector<Object *> &env){
	this->func_ptr = f_ptr;
	
	for(auto o : env){
		this->env.push_back(o);
	}
}

int Closure::get_func(void){
	return func_ptr;
}
