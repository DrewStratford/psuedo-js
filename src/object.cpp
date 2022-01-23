#include "object.hpp"

#include <iostream>

Object Object::object_list = Object();

Object::Object(){
	Object::object_list.insert_after(this);
}


std::ostream& Object::show(std::ostream& os) const{
	return os;
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

std::ostream& operator<<(std::ostream& os, const Object& obj){
	return obj.show(os);
}

template<>
std::ostream& ArrayList::show(std::ostream& os) const{
	os << "[";
	for(auto pos = begin(); pos < end(); pos++){
		ObjPtr o = *pos;
		o.show(os);
		if(pos + 1 != end())
			os << ", ";

	}
	return os << "]";
}

template<>
std::ostream& Dictionary::show(std::ostream& os) const{
	const Dictionary *hack = this;
	os << "{";
	for(auto pos = begin(); pos != hack->end(); pos++){
		auto pair = *pos;
		os << pair.first.c_str() << ": ";
		pair.second.show(os);
		os << (", ");
	}
	return os << ("}");
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
		if(Object* obj = o.as_o())
			obj->mark();
	}
}

std::ostream& Closure::show(std::ostream&) const{
	std::cout << "CLOSURE " << func_ptr;
}

int Closure::get_func(void){
	return func_ptr;
}

std::ostream& Closure::print_env(std::ostream& os) const{
	for(auto o : env){
		os << "\tcap: ";
		o.show(os);
	}
	return os;
}

void Closure::push_var(ObjPtr o){
	env.push_back(o);
}

// StringObject implementations
StringObject::StringObject(std::string&& str){
	this->str = str;
}

void StringObject::mark(void){
	active = true;
}

std::ostream& StringObject::show(std::ostream& os) const{
	return os << this->str;
}

StringObject* StringObject::add(StringObject* object){
	return new StringObject(this->str + object->str);
}

/*
 * ObjPtr constructors
 */

std::ostream& ObjPtr::show(std::ostream& os) {
	switch(type){
		case INT:
			os << as_i();
			break;
		case FLOAT:
			os << as_f();
			break;
		default:
		{
			Object *o= nullptr;
			if(o = as_o()){
				o->show(os);
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

ObjPtr::object_ptr(StringObject *c){
	this->type = STRING;
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
	if(type == DICT || type == CLOSURE || type == ARRAY || type == STRING)
		return (Object*)ptr;
	return nullptr;
}

Closure* ObjPtr::as_c(void){
	if(this->type == CLOSURE) return (Closure*)(data << 4);
	return nullptr;
}
ArrayList* ObjPtr::as_arr(void){
	if(this->type == ARRAY) return (ArrayList*)(data << 4);
	return nullptr;
}
Dictionary* ObjPtr::as_dict(void){
	if(this->type == DICT) return (Dictionary*)(data << 4);
	return nullptr;
}
StringObject* ObjPtr::as_string(void){
	if(this->type == STRING) return (StringObject*)(data << 4);
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
	if(a.type == STRING && b.type == STRING){
		return ObjPtr(a.as_string()->add(b.as_string()));
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
