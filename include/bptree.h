#ifndef BPTREE_H
#define BPTREE_H

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <queue>
#include <set>

#include "common.h"

#define SUCCESS       0
#define UNKNOWN_ERROR 1
#define FULL          2
#define SAME_KEY      3
#define KEY_NOT_FOUND 4
#define TOO_FEW_KEYS  5

#define CLASS_VALUE   1
#define CLASS_NODE    2

#define FLOOR(x, y) int( (x) / (y) )
#define CEIL(x, y)  int( (x - 1) / (y) + 1 )

using namespace std;

class bptree {
private:
	class bpentry {
	private:
		int type;
	public:
#ifdef DEBUG
		int id;
		static int global_id_counter;
#endif
		bpentry(int _type) { 

#ifdef DEBUG
			id = global_id_counter++;
#endif

			type = _type;
		}
		
		virtual ~bpentry() {}
		
		int getType() {
			return type;
		}
	};


	class bpvalue: public bpentry {
	private:
		itemptr * value;
	public:
		bpvalue(itemptr * _value) : bpentry(CLASS_VALUE),
			value(_value) {}
		inline itemptr * getvalue() { return value; }
	};


	class bpnode: public bpentry {
	public:
		vector< pair<itemkey, bpentry *> > pairs;
		// Note:
		//   If it is a leaf node, extra_entry is the
		//   pointer to next leaf node 
		//   If it is an interior node, extra_entry is
		//   the right most entry
		bpnode * extra_entry;    

		// The following 3 attributes are for the sake of easier
		// implementaion in insert() and remove()
		// John recommends to store this value
		bpnode * parent;    

		// bptree pointer
		bptree * tree;

		// Louis recommends to store this value
		bpnode * left_ptr;  

		// Louis recommends to store this value,
		// the same as extra_entry when this node is a leaf node
		bpnode * right_ptr; 

		// Constructor
		bpnode(bptree * _tree);
		// only for making new root
		bpnode(bptree * _tree, itemkey _key,
				bpnode * left, bpnode * right);

		// Deconstructor
		~bpnode();

		// Insert functions 
		int insert(itemkey _key, bpentry * _entry);
		int forceInsert(itemkey _key, bpentry * _entry);

		// Split function for full condition of nodes
		pair<itemkey, bpentry *> split(itemkey _key,
				bpentry * _entry);

		// Find functions
		bpentry * findChild(itemkey _key);
		bpentry * findValueEntry(itemkey _key);
		bpentry * findLeftMostChild();

		// Remove functions
		bool removeValueEntry(itemkey _key);
		void forceRemove();
		int remove(itemkey _key);

		// Static function for coalesce two sibling nodes
		static bool coalesce( bpnode * _left, bpnode * _right,
				bool merge_to_right = false);

		// Static function for redistribute two sibling nodes
		static bool redistribute(bpnode * _left, bpnode * _right,
				bool right_to_left = false);

		// Static function for checking if two nodes are siblings
		static bool isSibling(bpnode * _left, bpnode * _right);

		// Get the number of pointers in current node,
		// according to its node type
		int numOfEntries();

		// Check if current node have enough keys to maintain valid
		bool hasEnoughKeys();

		// Check if current node have extra entries to redistribute
		bool hasExtraKeys();

		// Check if current node is leaf node or interior node
		bool isLeaf();

		// Return extra_leaf pointer
		bpnode * getNextLeaf();

		// Set extra_leaf pointer
		void setNextLeaf(bpnode * _next_leaf);

		// Set up sibling relation [util function for insert]
		void becomeRightSibingOf(bpnode * _left);

		// Set up sibling relation [util function for insert]
		void becomeLeftSiblingOf(bpnode * _right);

		// Print current node's key values
		void printKeys();

		// Print current nodes's entry values
		void printValues();
	};

	bpnode * root;
	int height;

	// Function for make new bptree root while splitting
	void makeNewRoot(itemkey _key, bpnode * _left, bpnode * _right) {
		root = new bpnode(this, _key, _left, _right);
		height += 1;
	}

	// Function for reduce height when bpTree has too less items
	void deleteEmptybpnode(bpnode * current_node) {
		if(isRoot(current_node) && height > 1) {
			root = current_node->extra_entry;
			root->parent = NULL;
			height -= 1;
		}else {
#ifdef DEBUG
			cout << "bptree::deleteEmptybpnode" <<
				" - this is a single root, can't be" <<
				" deleted" << endl;
#endif 
		}
	}

public:
	bool isRoot(bpnode * _root) { 
		return ( _root == root );
	}

	int node_number;
	int key_num;

	// Constructor
	bptree(int _key_num);
	// Copy constructor
	bptree(const bptree& _copy);
	// Deconstructor
	~bptree();

	// Desired assignment overrides
	bptree& operator =(const bptree& _other);
	// Desired interfaces for bptree
	bool insert(itemkey _key, itemptr& _value);
	bool remove(itemkey _key);
	itemptr * find(itemkey _key);
	void printKeys();
	void printValues();

};


#endif 
// BPTREE_H
