#ifndef MY_LIST_H
#define MY_LIST_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* pointer to compare function, needed for sorting */
typedef bool (*node_compare)(void*, void*);

class list_t{
private:
	struct node_t {
		void* data;
		node_t* next;
		node_t(void* vdata) : data(vdata), next(nullptr) {}
	};
	node_t* _head;
public:
	class list_iterator{
	private:
		node_t* curr;
	public:
		list_iterator(node_t* node) : curr(node) {}
		bool operator != (const list_iterator& other) const{
			return curr != other.curr;
		}
		bool operator == (const list_iterator& other) const{
			return curr == other.curr;
		}
		void* operator*() const {
			return curr->data;
		}
		list_iterator operator++() {
			curr = curr->next;
			return *this;
		}
	};
	list_t() : _head(nullptr) {}
	~list_t() {	clear(); }
	void clear();
	int push_back(void* vdata);
	void* head();
	void* back();
	void* pop();
	void* pop_back();
	void pop_item(void* data);
	void bubble_sort(node_compare func);

	list_iterator begin() const {
		return list_iterator(_head);
	}
	list_iterator end() const {
		return list_iterator(nullptr);
	}
};

#ifdef __cplusplus
}
#endif

#endif
