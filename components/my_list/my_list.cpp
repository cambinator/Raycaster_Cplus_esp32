#include "my_list.hpp"

void list_t::clear(){
	while(_head != nullptr){
		node_t* temp = _head;
		_head = _head->next;
		delete temp;
	}
}

int list_t::push_back(void* vdata)
{
	node_t* new_node = new node_t(vdata);
	if (new_node == nullptr){
		return 1;
	}
	if (_head == nullptr){
		_head = new_node;
	} else {
		node_t* temp = _head;
		while(temp->next != nullptr){
			temp = temp->next;
		}
		temp->next = new_node;
	}
	return 0;
}

void* list_t::head ()
{
	if (_head == nullptr){
		return nullptr;
	}
	return _head->data;
}

void* list_t::back()
{
	if (_head == nullptr) {
		return nullptr;
	}
	node_t* temp = _head;
	while(temp->next != nullptr){
		temp = temp->next;
	}
	return temp->data;
}

void* list_t::pop()
{
	if (_head == nullptr) {
		return nullptr;
	}
	node_t* temp = _head;
	_head = _head->next;
	void* data = temp->data;
	delete temp;
	return data;
}

void* list_t::pop_back()
{
	if (_head == nullptr) {
		return nullptr;
	}
	node_t* temp = _head;
	node_t* previous = nullptr;
	while(temp->next != nullptr){
		previous = temp;
		temp = temp->next;
	}
	if (previous){
		previous->next = nullptr;
	} else {
		_head = nullptr;
	}
	void *data = temp->data;
	delete temp;
	return data;
}


void list_t::pop_item(void* data)
{
	if (_head == nullptr){
		return;
	}
	node_t*  temp;
	if (_head->data == data){
		temp = _head;
		_head = _head->next;
		delete temp;
		return;
	}
	node_t* curr = _head;
	while (curr->next != nullptr && curr->next->data != data){
		curr = curr->next;
	}
	if (curr->next != nullptr)
	{
		temp = curr->next;
		curr->next = curr->next->next;
		delete temp;
	}
	return;
}

void list_t::bubble_sort(node_compare func)
{
	int check = 1;
	while(check){
		node_t* curr = _head;
		if (curr == nullptr) {
			return;
		}
		node_t* next = _head->next;
		void* temp = nullptr;
		check = 0;
		while(next != nullptr){
			if (func(curr->data, next->data)){
				temp = curr->data;
				curr->data = next->data;
				next->data = temp;
				check = 1;
			}
			curr = next;
			next = next->next;
		}
	}
}

