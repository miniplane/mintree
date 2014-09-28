#include "Container.h"
#include <iostream>
#define nullptr NULL // remove if c++11

class Empty_Tree_Exception : public ContainerException {
public:
	virtual const char * what() const throw () { return "BpTree: Tree is empty"; }
};

template <typename E>
inline void shift(E* array, size_t begin, size_t end, int shift) {
	if (shift > 0)
		for (size_t i = end-1; i>=begin+shift; i--)
			array[i] = array[i-shift];

	else if (shift < 0)
		for (size_t i = begin; i<end+shift; i++)
			array[i] = array[i-shift];
}

template <typename E, size_t k=2>
class BpTree : public Container<E> {

	class Node {
	public:

		static const size_t order = k*2;
		Node*  parent;
		Node*  children[order+1];
		Node*  next_node;
		E      keys[order];
		size_t elements; // not higher than order-1
		bool   leaf;

		Node(Node* parent_ = nullptr) : parent(parent_) {
			elements = 0;
			leaf = true;
			next() = previous() = nullptr;
		}

		~Node () {
			if (leaf)
				return;
			for (size_t i = 0; i < elements+1; i++)
				delete children[i];
		}

		inline Node*& next() { // returns last children pointer (for leafs); used to point to next leaf
			return children[order];
		}

		inline Node*& previous() {
			return children[0];
		}

		std::ostream& print (std::ostream& o, int depth) const {
			for (int j = 0; j < depth; j++)
				o << "  ";

			if (leaf)
				o << "leaf : " << this << std::endl;
			else
				o << "inner: " << this << std::endl;
			for (size_t i = 0; i < elements; i++) {
				if (!leaf)
					children[i]->print(o, depth+1);
				for (int j = 0; j < depth; j++) {
					o << "  ";				// one space per depth
				}
				o << "  " << keys[i] << std::endl;
			}
			if (!leaf)
				children[elements]->print(o, depth+1); // calls print for children
			return o;
		}

		bool apply_(const Functor<E>& f, Order ord, size_t& n) { // I'm not sure yet why we need that, but ok

			size_t i;
			if (ord == descending) {
				i = elements-1;
				if (!leaf)
					if (!children[i+1]->apply_(f, ord, n))
						return false;
				do {
					if (i > elements) break;
					if (leaf) {
						n++; // counts values in leafs
						if (!f(keys[i]))
							return false;
						}
					else 
						if (!children[i]->apply_(f, ord, n)) // goes to next child until leaf
							return false;
				} while (i--);
			}

			else {
				i = 0;
				for (; i < elements; i++) {
					if (leaf) {
						n++; // counts values in leafs
						if (!f(keys[i]))
							return false;
						}
					else 
						if (!children[i]->apply_(f, ord, n)) // goes to next child until leaf
							return false;
				}
				if (!leaf)
					if (!children[i]->apply_(f, ord, n))
						return false;
			}
			return true;
		}

		size_t num_keys() { // returns the num of keys in the leafs reachable from the current node (size of tree)
			if (leaf)
				return elements;
			
			size_t sum = 0;
			for (size_t i = 0; i < elements+1; i++)
				sum += children[i]->num_keys();

			return sum;
		}

		E& min() {
			if (leaf)
				return keys[0];
			else
				return children[0]->min();
		}

		E& max() {
			if (leaf)
				return keys[elements-1];
			else
				return children[elements]->max();
		}

		bool contains(const E& key) const {
			// returns true if value(key) is in tree
			size_t i;

			for (i = 0; i < elements; i++) {
				if (keys[i] > key) // key isn't in this node
					break;
				else if (keys[i] == key) 
					return true;
			}

			if (leaf) // key isn't in tree
				return false;

			return children[i]->contains(key); // if node, check child for key
		}

		Node *insert_in_leaf(E key, size_t i) {
			for (size_t j=elements; j > i; j--)
				keys[j] = keys[j-1]; // moves elements
			
			keys[i] = key; // add new element
			elements++;

			if (elements > order-1) // if node is full, split it
				return split(); // make new child

			return nullptr;
		}

		Node *insert_in_node(E key, size_t i) {

			Node *right_child = children[i]->insert(key); // if key smaller than value move it to child
			if (right_child == nullptr)	// nothing is split
				return nullptr;

			for (size_t j=elements; j > i; j--) {
				keys[j] = keys[j-1]; // moves elements
				children[j+1] = children[j]; // moves pointers
			}
			
			keys[i] = lift_key(children[i], right_child); // add new element
			elements++;

			children[i+1] = right_child;

			if (elements > order-1) // if node is full, split it
				return split(); // make new child

			return nullptr;
		}

		Node *insert(E key) { // insertions into inner nodes and leafs

			size_t i = 0;

			for (i = 0; i < elements; i++) {
				if (key == keys[i])
					return nullptr; // return to avoid inserting the same value twice

				if (keys[i] > key) // if key smaller than value, stop
					break;
			}

			Node *retval = nullptr;

			if (leaf)
				retval = insert_in_leaf(key, i);
			else
				retval = insert_in_node(key, i);
			return retval;
		}

		Node *split() { // splits one Node in two

			size_t splitpoint = elements/2;

			Node * const left = this;
			Node *right = new Node(parent);

			right->leaf = left->leaf; // right is the same (inner or leaf) as left
			int z = leaf == false;

			for (size_t j = splitpoint+z; j < elements+1; j++)  {
				if (!leaf) {
					left->children[j]->parent = right; // copys children (one child more than value)
					right->children[right->elements] = left->children[j];
				}
				if (j<elements) {
					right->keys[right->elements] = left->keys[j]; // copies values
					right->elements++;
				}
			}

			left->elements = splitpoint; // sets new value for elements

			if (leaf) { // resets pointers to next and previous children
				right->next() = left->next();
				left->next() = right;
				right->previous() = left; 
				if (right->next() != nullptr)
					right->next()->previous() = right; 
			}
			return right;
		}

		inline bool can_borrow_from(Node* n) { // tells if node is full enough to give val to sibling
			return n && (n->parent == parent) && (n->elements > k);
		}

		inline E pop_left() { // removes leftest key
			E key = keys[0];
			shift(keys, 0, elements--, -1);
			return key;
		}

		inline void push_left(E key) { // inserts key left
			shift(keys, 0, ++elements, 1);
			keys[0] = key;
		}

		inline E pop_right() { // removes rightest key
			return keys[--elements];
		}

		inline void push_right(E key) { // inserts key right
			keys[elements++] = key;
		}

		bool has_no_child(const Node* child) const { // checks if children from Node were removed
			for (size_t i=0; i<elements+1; i++)
				if (children[i] == child)
					return false;
			return true;
		}

		void set_key_in_node(size_t index, E key) { // sets key in node to new min of right branch
			
			if (index > 0)
				keys[index-1] = key;
			else if (parent) {
				size_t pos_in_parent = 0;
				for (; pos_in_parent < parent->elements+1; pos_in_parent++)
					if (this == parent->children[pos_in_parent])
						break;
				parent->set_key_in_node(pos_in_parent, key);
			}
		}

		void remove_from_leaf(size_t value_pos, size_t pos_in_parent) {

			shift(keys, value_pos, elements, -1); // moves element to left, removes key
			elements--;

			if (!parent)
				return;
			
			parent->set_key_in_node(pos_in_parent, keys[0]);

			if (elements >= k) // if Node at least half full, or root, return
				return;

			// tries to borrow value from sibling
			if (can_borrow_from(previous())) {
				push_left(previous()->pop_right());
				parent->set_key_in_node(pos_in_parent, keys[0]); // update next()'s key in parent
				return;
			}
			else if (can_borrow_from(next())) {
				push_right(next()->pop_left());
				parent->set_key_in_node(pos_in_parent+1, next()->keys[0]); // update next()'s key in parent
				return;
			}

			// if borrowing not possible -> merge
			if (previous() && (previous()->parent==parent)) {
				previous()->merge_leaf(pos_in_parent-1);
				delete this;
				return;
			}
			else if (next() && (next()->parent==parent)) {
				Node *n = next();
				merge_leaf(pos_in_parent);
				delete n;
				return;
			}
		}

		void merge_leaf(size_t pos_in_parent) { // if root empty, merged children form new root

			for (size_t i = 0; i < next()->elements; i++) 
				push_right(next()->keys[i]); // all values from next() are appended to this

			for (size_t i = pos_in_parent+1; i < parent->elements; i++) // moves children from parent node
				parent->children[i] = parent->children[i+1];

			for (size_t i = pos_in_parent; i < parent->elements-1; i++)
				parent->keys[i] = parent->keys[i+1]; // deletes value in parent pointing to child (min value of current node)

			parent->elements--;
			next()->unlink();

			parent->after_removal_from_inner();
		}

		void after_removal_from_inner() {
			if (elements >= k)
				return; // everything's fine

			if (!parent)
				return; // no siblings

			size_t pos_in_parent;
			for (pos_in_parent = 0; pos_in_parent <= parent->elements; pos_in_parent++) // finds position of this inner node
				if (this == parent->children[pos_in_parent])
					break;

			Node* prev_inner = pos_in_parent > 0                ? parent->children[pos_in_parent-1] : nullptr;
			Node* next_inner = pos_in_parent < parent->elements ? parent->children[pos_in_parent+1] : nullptr;

			// inner tries to borrow key from sibling

			if (can_borrow_from(prev_inner)) // borrow from previous
				return borrow_from_node(prev_inner, pos_in_parent, true);
			else if (can_borrow_from(next_inner)) // borrow from next
				return borrow_from_node(next_inner, pos_in_parent, false);
			
			// if borrowing not possible, -> merge

			if (prev_inner)
				prev_inner->merge_nodes(this, pos_in_parent-1); // call from previous, pass current
			else if (next_inner)
				merge_nodes(next_inner, pos_in_parent); // call from current, pass next
			else
				return;
		}

		void borrow_from_node(Node* other, size_t pos_in_parent, bool borrow_from_previous) {

			if (borrow_from_previous) {
				push_left(parent->keys[pos_in_parent-1]); // inserts val from parent at the beginning

				for (size_t i = elements; i > 0; i--) // inserts child at beginning
					children[i] = children[i-1];

				children[0] = other->children[other->elements]; // moves child from left to right
				children[0]->parent = this;
				parent->keys[pos_in_parent-1] = other->pop_right(); // puts key from left in parent
			}
			else { // borrow from next
				push_right(parent->keys[pos_in_parent]); // inserts val from parent at the end
				children[elements] = other->children[0]; // adds child from right node
				children[elements]->parent = this;

				for (size_t i = 0; i < other->elements; i++) // rearrange children from other node
					other->children[i] = other->children[i+1];
				
				parent->keys[pos_in_parent] = other->pop_left();
			}
		}

		void merge_nodes(Node* other, size_t pos_in_parent) { // always called from previous

			push_right(parent->keys[pos_in_parent]);

			size_t i;
			for (i = 0; i < other->elements; i++) {
				children[elements] = other->children[i];
				children[elements]->parent = this;
				push_right(other->keys[i]); // all values from next() are appended to this
			}

			children[elements] = other->children[i];
			children[elements]->parent = this;

			// parent is in inconsistent state here

			for (size_t i = pos_in_parent; i < parent->elements-1; i++) {// moves children from parent node
				parent->keys[i] = parent->keys[i+1];
				parent->children[i+1] = parent->children[i+2];
			}

			parent->elements--;
			other->elements = -1;
			delete other;
			parent->after_removal_from_inner();
		}

		void announce_self() { // sets next or previous pointer from surrounding nodes as self
			if (next())     next()->previous() = this;
			if (previous()) previous()->next() = this;
		}

		void unlink() { // resets the previous & next pointers in leafs
			if (next()) next()->previous() = previous();
			if (previous()) previous()->next() = next();			
		}

		void remove(E key, size_t pos_in_parent) {

			if (leaf) {
				for (size_t i=0; i < elements; i++) {
					if (key == keys[i]) {
						remove_from_leaf(i, pos_in_parent);
						break;
					} // else, loop ends without anything being removed
				}
			} 
			else {
				size_t i;
				for (i=0; i < elements; i++)
					if (keys[i] > key)
						break;

				children[i]->remove(key, i);
				// don't place ANYTHING here
			}
		}
	};

	static const E& lift_key(const Node* left, const Node* right) { // returns key that goes to parent Node after split
		if (left->leaf)
			return right->keys[0];
		else
			return left->keys[left->elements];
	}

	Node *root;
	bool member_( Node* node, const E& e ) const;

public:

	BpTree () : root(new Node(nullptr)) {}

	~BpTree () {
		delete root;
	}

	virtual void add(const E& e) {
		Node *right_subroot = root->insert(e);
		if (right_subroot != nullptr) { // if root is split, make new root
			
			Node *left_subroot = root;

			root = new Node(nullptr);
			root->leaf = false;
			root->elements = 1;

			root->keys[0] = lift_key(left_subroot, right_subroot);

			root->children[0] = left_subroot;  left_subroot->parent = root; // resets parents and children
			root->children[1] = right_subroot; right_subroot->parent = root;
		}
	}

	virtual std::ostream& print( std::ostream& o ) const { return root->print(o, 0); }

	virtual void remove(const E& e ) {
		root->remove(e, 0);
		if (!root->leaf && root->elements == 0) {
			Node *oldroot = root;
			root = root->children[0];
			root->parent = nullptr;
			oldroot->elements = -1;
			delete oldroot;
		}
	}

	virtual bool member(const E& e) const { return root->contains(e); }

	virtual void add( const E e[], size_t s ) {
		for (size_t i=0; i < s; i++)
			add(e[i]);
	}

	virtual void remove (const E e[], size_t s) {
		for (size_t i=0; i < s; i++)
			remove(e[i]);
	}

	virtual size_t apply (const Functor<E>& f, Order order_=dontcare) const {
		if (root->elements == 0)
			return 0;
		size_t n = 0;
		root->apply_(f, order_, n);
		return n;
	}

	virtual size_t size() const { return root->num_keys(); }

	virtual E min() const { 
		if (root->elements == 0)
			throw Empty_Tree_Exception();
		return root->min();
	}

	virtual E max() const { 
		if (root->elements == 0)
			throw Empty_Tree_Exception();
		return root->max(); 
	}
};
