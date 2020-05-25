#include "object.hpp"

#include <iostream>

Object Object::object_list = Object();

Object::Object(){
	Object::object_list.insert_after(this);
}


void Object::show(void){
	return;
}
void Object::mark(void){
	this->active = true;
}

/*
 * Garbage collection stuff
 */

void gc_sweep_vector(std::vector<ObjPtr> &vec){
	for(auto o : vec){
		Object *obj = nullptr;
		if(obj = o.as_o()){
			obj->mark();
		}
	}
}

void gc_sweep_map(std::map<std::string, ObjPtr> &map){
	for(auto p : map){
		ObjPtr o = p.second;
		Object *obj = nullptr;
		if(obj = o.as_o()){
			obj->mark();
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

template<>
void ArrayList::mark(void){
	if(active) return;
	active = true;
	for(auto pos = begin(); pos < end(); pos++){
		ObjPtr o = *pos;
		Object *obj = nullptr;
		if(obj = o.as_o()){
			obj->mark();
		}
	}
}

template<>
void ArrayList::show(void){
	printf("[");
	for(auto pos = begin(); pos < end(); pos++){
		ObjPtr o = *pos;
		o.show();
		if(pos + 1 != end())
			printf(", ");

	}
	printf("]");
}

template<>
void Dictionary::show(void){
	Dictionary *hack = this;
	printf("{");
	for(auto pos = begin(); pos != hack->end(); pos++){
		auto pair = *pos;
		printf("%s : ", pair.first.c_str());
		pair.second.show();
		printf(", ");
	}
	printf("}");
}

template<>
void Dictionary::mark(void){
	if(active) return;
	active = true;

	Dictionary *hack = this;
	for(auto pos = begin(); pos != hack->end(); pos++){
		auto pair = *pos;
		Object *obj = nullptr;
		if(obj = pair.second.as_o()){
			obj->mark();
		}
	}
}

/////////////////////////////////////////////////////
// Closure stuff.
/////////////////////////////////////////////////////

Closure::Closure(int f_ptr){
	func_ptr = f_ptr;
}

void Closure::mark(void){
	if(active) return;
	active = true;
	for(auto o : env){
		o.show();
	}
}

void Closure::show(void){
	std::cout << "CLOSURE " << func_ptr;
}

int Closure::get_func(void){
	return func_ptr;
}

void Closure::print_env(void){
	for(auto o : env){
		printf("\tcap: ");
		o.show();
	}
}

void Closure::push_var(ObjPtr o){
	env.push_back(o);
}

/*
 * ObjPtr constructors
 */

void ObjPtr::show(void){
	switch(type){
		case INT:
			printf("%d", as_i());
			break;
		case FLOAT:
			printf("%f", as_f());
			break;
		default:
		{
			Object *o= nullptr;
			if(o = as_o()){
				o->show();
			}
		}
	}
}

ObjPtr::object_ptr(void){
	this->type = INT; //TODO: UNIT?
	this->data = 0;
}

ObjPtr::object_ptr(int32_t i){
	this->type = INT;
	this->data = i;
}

ObjPtr::object_ptr(float f){
	this->type = FLOAT;
	this->data = f;
}

ObjPtr::object_ptr(Closure *c){
	this->type = CLOSURE;
	this->data = (int64_t)c >> 4;
}

ObjPtr::object_ptr(Dictionary *c){
	this->type = DICT;
	this->data = (int64_t)c >> 4;
}

ObjPtr::object_ptr(ArrayList *c){
	this->type = ARRAY;
	this->data = (int64_t)c >> 4;
}

int32_t ObjPtr::as_i(void){
	if(this->type == INT) return (int32_t)this->data;
	return 0;
}

float ObjPtr::as_f(void){
	if(this->type == FLOAT) return (float)this->data;
	return 0;
}

Object *ObjPtr::as_o(void){
	void* ptr = (void*)(data << 4);
	if(type == DICT || type == CLOSURE || type == ARRAY)
		return (Object*)ptr;
	return nullptr;
}

Closure *ObjPtr::as_c(void){
	if(this->type == CLOSURE) return (Closure*)(data << 4);
	return nullptr;
}
ArrayList *ObjPtr::as_arr(void){
	if(this->type == ARRAY) return (ArrayList*)(data << 4);
	return nullptr;
}
Dictionary *ObjPtr::as_dict(void){
	if(this->type == DICT) return (Dictionary*)(data << 4);
	return nullptr;
}

/*
 * Arithmetic etc
 */
ObjPtr add(ObjPtr a, ObjPtr b){
	ArrayList *arr = nullptr;
	if(a.type == INT && b.type == INT) 
		return  ObjPtr((int32_t) (a.as_i() + b.as_i()));
	if(a.type == FLOAT && b.type == FLOAT) 
		return  ObjPtr(a.as_f() + b.as_f());
	if(arr = a.as_arr()){
		arr->push_back(b);
		return a;
	}
	if(arr = b.as_arr()){
		arr->insert(arr->begin(), a);
		return b;
	}
	return  ObjPtr();
}

ObjPtr sub(ObjPtr a, ObjPtr b){
	if(a.type == INT && b.type == INT) 
		return ObjPtr(a.as_i() - b.as_i());
	if(a.type == FLOAT && b.type == FLOAT) 
		return ObjPtr(a.as_f() - b.as_f());
	return ObjPtr();
}

ObjPtr mul(ObjPtr a, ObjPtr b){
	if(a.type == INT && b.type == INT) 
		return ObjPtr(a.as_i() * b.as_i());
	if(a.type == FLOAT && b.type == FLOAT) 
		return ObjPtr(a.as_f() * b.as_f());
	return ObjPtr();
}

ObjPtr div(ObjPtr a, ObjPtr b){
	if(a.type == INT && b.type == INT) 
		return ObjPtr(a.as_i() / b.as_i());
	if(a.type == FLOAT && b.type == FLOAT) 
		return ObjPtr(a.as_f() / b.as_f());
	return ObjPtr();
}

ObjPtr mod(ObjPtr a, ObjPtr b){
	if(a.type == INT && b.type == INT) 
		return  ObjPtr(a.as_i() % b.as_i());
	return  ObjPtr();
}

ObjPtr lt(ObjPtr a, ObjPtr b){
	if(a.type == INT && b.type == INT) 
		return ObjPtr(a.as_i() < b.as_i());
	if(a.type == FLOAT && b.type == FLOAT)
		return ObjPtr(a.as_f() < b.as_f());
	return ObjPtr();
}

ObjPtr lte(ObjPtr a, ObjPtr b){
	if(a.type == INT && b.type == INT) 
		return ObjPtr(a.as_i() <= b.as_i());
	if(a.type == FLOAT && b.type == FLOAT) 
		return ObjPtr(a.as_f() <= b.as_f());
	return ObjPtr();
}

ObjPtr gt(ObjPtr a, ObjPtr b){
	if(a.type == INT && b.type == INT) 
		return ObjPtr(a.as_i() > b.as_i());
	if(a.type == FLOAT && b.type == FLOAT) 
		return ObjPtr(a.as_f() > b.as_f());
	return ObjPtr();
}

ObjPtr gte(ObjPtr a, ObjPtr b){
	if(a.type == INT && b.type == INT) 
		return ObjPtr(a.as_i() >= b.as_i());
	if(a.type == FLOAT && b.type == FLOAT) 
		return ObjPtr(a.as_f() >= b.as_f());
	return ObjPtr();
}


/*
#include <cstdio>
int main(){
	printf("hello\n");
	ObjPtr a = ObjPtr(10);
	ObjPtr b = ObjPtr(22);
	ObjPtr c = add(a, b);
	printf("%d\n", a.as_i());
	printf("%d\n", b.as_i());
	printf("10 + 22 = %d\n", c.as_i());
	ObjPtr clos = ObjPtr(new Closure(0));
	clos.as_o()->show();
	ArrayList* arr = new ArrayList();
	Dictionary* dict = new Dictionary();
	dict->emplace("foo", c);
	arr->push_back(a);
	arr->push_back(clos);
	arr->push_back(b);
	arr->mark();
	arr->show();
	dict->show();
	return 0;
}
*/
