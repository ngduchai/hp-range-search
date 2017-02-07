#include "bptree.h"


bptree::bpnode::bpnode(bptree * _tree) : bpentry(CLASS_NODE) {
	pairs.clear();

	extra_entry = NULL; 

	parent = NULL;
	left_ptr = NULL;
	right_ptr = NULL;

	tree = _tree;
	tree->node_number += 1;
	// static member access
}


bptree::bpnode::bpnode(bptree * _tree, itemkey _key, bpnode * left,
		bpnode * right) : bpentry(CLASS_NODE) {
	// only for making new root
	pairs.clear();

	pairs.push_back( make_pair(_key, (bpentry *)left) );
	extra_entry = right;


	left->parent = this;
	right->parent = this;
	parent = NULL;

	left_ptr = NULL;
	right_ptr = NULL;

	right->becomeRightSibingOf(left);

	tree = _tree;
	tree->node_number += 1;
}

bptree::bpnode::~bpnode() {}

bool bptree::bpnode::isLeaf() {
	return ( tree->node_number == 1 ) ||
		(pairs[0].second->getType() == CLASS_VALUE);
}

void bptree::bpnode::setNextLeaf(bpnode * _next_leaf) {
	extra_entry = _next_leaf;
}

void bptree::bpnode::becomeRightSibingOf(bpnode * _left) {
	if (_left != NULL) _left->right_ptr = this;
	this->left_ptr = _left;
}
void bptree::bpnode::becomeLeftSiblingOf(bpnode * _right){
	if (_right != NULL) _right->left_ptr = this;
	this->right_ptr = _right;
}

bptree::bpnode * bptree::bpnode::getNextLeaf() {
	return extra_entry;
}

void bptree::bpnode::printKeys() {

#ifdef DEBUG
	printf("  %d,p:%d,l:%d,r:%d,ks:", 
			id, 
			parent == NULL ? 0 : parent->id, 
			left_ptr == NULL ? 0 : left_ptr->id, 
			right_ptr == NULL ? 0 : right_ptr->id);
#endif

	printf("[");
	for (auto it = pairs.begin(); it != pairs.end(); it++) {
		printf("%ld-", *(uint64_t*)it->first.value());
		printf("%ld  ", ((uint64_t*)it->first.value())[1]);
	}
	printf("]");

#ifdef DEBUG
	if (!isLeaf()) {
		printf(",cld:[");
		for (auto it = pairs.begin(); it != pairs.end(); it++) {
			if (it == pairs.begin()) printf("%d", it->second->id);
			else printf(",%d", it->second->id);
		}
		printf(",%d", extra_entry->id);
		printf("]");
	}
	else {
		printf(",ext:%d", extra_entry == NULL ? 0 : extra_entry->id);
	}
#endif
	printf(" ");

}

void bptree::bpnode::printValues() {
#ifdef DEBUG
	printf("%d,%d,", id, parent == NULL ? 0 : parent->id);
#endif

	printf("[");
	for (auto it = pairs.begin(); it != pairs.end(); it++) {
		printf("%ld-", *(uint64_t*)
			(((bpvalue *)it->second)->getvalue()->skey.value()));
		printf("%ld  ", ((uint64_t*)(((bpvalue *)
			it->second)->getvalue()->skey.value()))[1]);
	}
	printf("] ");
}

bptree::bpentry * bptree::bpnode::findChild(itemkey _key) {
	for (auto it = pairs.begin(); it != pairs.end(); it++) {
		if ( it->first > _key ) {
			return it->second;
		}
	}
	return extra_entry;
}

bptree::bpentry * bptree::bpnode::findValueEntry(itemkey _key) {
	for (auto it = pairs.begin(); it != pairs.end(); it++) {
		if ( it->first == _key ) {
			return it->second;
		}
	}
	return NULL;
}

bptree::bpentry * bptree::bpnode::findLeftMostChild() {
	if( pairs.size() > 0 )
		return pairs.front().second;
	else
		return NULL;
}

int bptree::bpnode::insert(itemkey _key, bpentry * _entry) {
	if ((int)pairs.size() < tree->key_num) {
		return forceInsert(_key, _entry);
	}
	else {
		return FULL; // full
	}
}


int bptree::bpnode::forceInsert(itemkey _key, bpentry * _entry) {
	for (auto it = pairs.begin(); it != pairs.end(); it++) {
		if ( _key < it->first ) {
			pairs.insert(it, make_pair(_key, _entry));
			if ( !isLeaf() ) ((bpnode *) _entry)->parent = this;
			return SUCCESS;
		}
		else if (_key == it->first) {
			return SAME_KEY;
		} 
	}
	pairs.push_back( make_pair(_key, _entry) );
	if ( !isLeaf() ) ((bpnode *) _entry)->parent = this;
	return SUCCESS;
}


pair<itemkey, bptree::bpentry *> bptree::bpnode::split(itemkey _key,
		bpentry * _entry) {
	int insert_status = forceInsert(_key, _entry);
	
	// didn't insert anything
	if (insert_status == SAME_KEY)
		return pair<itemkey, bpentry *>(itemkey(), NULL); 
	
	// from: temp_left_ptr <---> this
	// to:   temp_left_ptr <---> left_node <---> this
	bpnode * left_node = new bpnode(tree);

	bpnode * temp_left_ptr = left_ptr;
	left_node->becomeRightSibingOf( temp_left_ptr );
	becomeRightSibingOf( left_node );

	itemkey new_key;
	if (isLeaf()) {
		if (temp_left_ptr != NULL)
			temp_left_ptr->setNextLeaf(left_node);
		left_node->setNextLeaf( this );
		new_key = pairs[ (pairs.size()+1)/2 ].first;
	}
	else {
		new_key = pairs[ pairs.size()/2 ].first;
	}

	for (auto it = pairs.begin(); it != pairs.end(); it++) {
		if (it->first != new_key) {
			left_node->pairs.push_back( *it );
			if ( !isLeaf() )
				((bpnode *)it->second)->parent = left_node;
		}
		else {
			if ( !isLeaf() ) {
				left_node->extra_entry =
					(bpnode *)(it->second);
				left_node->extra_entry->parent = left_node;
				it++;
			}
			pairs.erase(pairs.begin(), it);
			break;
		}
	}

	return make_pair(new_key, (bpentry *)left_node);
}

int bptree::bpnode::numOfEntries() {
	if( isLeaf() )
		return ( pairs.size() );
	else
		return ( pairs.size() + int(extra_entry != NULL) ); 
}

bool bptree::bpnode::hasEnoughKeys() {

	//  #ifdef DEBUG
	//   cout << "bpnode::hasEnoughKeys - isLeaf(): " << isLeaf() << endl;
	//   cout << "bpnode::hasEnoughKeys - numOfEntries(): " << numOfEntries() << endl;
	//   cout << "bpnode::hasEnoughKeys - ( numOfEntries() >= FLOOR(tree->key_num + 1, 2) ): " << (numOfEntries() >= FLOOR(tree->key_num + 1, 2)) << endl;
	//   cout << "bpnode::hasEnoughKeys - ( numOfEntries() >= CEIL(tree->key_num + 1, 2) ): " << ( numOfEntries() >= CEIL(tree->key_num + 1, 2) ) << endl;
	// #endif

	if( isLeaf() )
		return ( numOfEntries() >= FLOOR(tree->key_num + 1, 2) );
	else
		return ( numOfEntries() >= CEIL(tree->key_num + 1, 2) );
}

bool bptree::bpnode::hasExtraKeys() {
	if( isLeaf() )
		return ( numOfEntries() > FLOOR(tree->key_num + 1, 2) );
	else
		return ( numOfEntries() > CEIL(tree->key_num + 1, 2) );
}

bool bptree::bpnode::isSibling(bpnode * _left, bpnode * _right) {
	if( _left == NULL || _right == NULL) 
		return false;

	// #ifdef DEBUG
	//     cout << "_left->parent's 1st child key: " << _left->parent->pairs.front().first 
	//          << "_right->parent's 1st child key: " << _right->parent->pairs.front().first << endl;
	//     cout << "bpnode::isSibling - _left->parent == _right->parent " << ((_left->parent == _right->parent )?"True":"False") << endl;
	// #endif 

	return ( _left->parent == _right->parent );
}

bool bptree::bpnode::removeValueEntry(itemkey _key) {
	auto it = pairs.begin();
	while(it != pairs.end())
	{
		if ( it->first == _key)
		{
			pairs.erase(it);
			return true;
		}
		it++;
	}

	// Note:
	// since we already introduced an external memory management tool 
	// for entries, set pointer to an unused entry is good enough, and 
	// may not cause issues like memory leak in the end of the program
	// execution

	return false;
}

int bptree::bpnode::remove(itemkey _key) {

	if( isLeaf() && removeValueEntry(_key) == false ){
		return KEY_NOT_FOUND;
	}

	if( !hasEnoughKeys() )
		return TOO_FEW_KEYS;

	return SUCCESS;
}

bool bptree::bpnode::redistribute( bpnode * _left, bpnode * _right,
		bool right_to_left) {
	if( _left->parent != _right->parent )
		return false; 

	auto parent = _right->parent;

	if( _left->isLeaf() && _right->isLeaf() ) {
		// For leaf node, redistribution may be easier

		if( right_to_left ){
			// Leaf node - Right to Left

			auto entry = _right->pairs.front();
			_right->pairs.erase(_right->pairs.begin());
			_left->pairs.push_back(entry);

		}
		else {
			// Leaf node - Left to Right
			auto entry = _left->pairs.back();
			_left->pairs.pop_back();
			_right->pairs.insert( _right->pairs.begin(), entry);
		}

		// Redistribute higher-level keys
		for(auto it = parent->pairs.begin(); it != parent->pairs.end(); it++) {
			if( it->second == _left ){
				it->first = _right->pairs.front().first;
			}
		}

	}
	else {
		itemkey key; 
		for(auto it = parent->pairs.begin(); it != parent->pairs.end(); it++) {
			if( it->second == _left ){
				key = it->first;
			}
		}
		// cout << "bpnode::redistribute - Key value: " << key << endl;
		if( right_to_left ){
			auto entry = _right->pairs.front(); 

			auto new_entry = make_pair( key, _left->getNextLeaf());
			_left->pairs.push_back(new_entry);

			_right->pairs.erase(_right->pairs.begin());

			// Set the parent of left entry
			((bpnode *)entry.second)->parent = _left;
			_left->setNextLeaf( (bpnode *) entry.second );

			// Redistribute higher-level keys
			for(auto it = parent->pairs.begin(); it != parent->pairs.end(); it++) {
				if( it->second == _left ){
					// it->first = _right->pairs.front().first;
					it->first = entry.first;
				}
			}
		}
		else {
			// Left to Right
			auto entry = _left->pairs.back(); 
			auto new_entry = make_pair(key, _left->getNextLeaf());

			_left->setNextLeaf( (bpnode *) entry.second);
			_left->pairs.pop_back();

			// Set the parent of right entry
			((bpnode *)new_entry.second)->parent = _right;
			_right->pairs.insert( _right->pairs.begin(), new_entry);

			// Redistribute higher-level keys
			for(auto it = parent->pairs.begin(); it != parent->pairs.end(); it++) {
				if( it->second == _left ){
					it->first = entry.first;
				}
			}
		}

	}

	return true;
}
void bptree::bpnode::forceRemove() {
	this->pairs.clear();
	this->tree->node_number--;
	this->left_ptr    = NULL;
	this->right_ptr   = NULL;
}

bool bptree::bpnode::coalesce( bpnode * _left, bpnode * _right,
		bool merge_to_right) {
	// Function phylosophy
	// ==========================================================
	// 1. Merge bpnodes
	// 2. Delete Parent Key
	// 3. Rebalance Parent Key
	if( _left->parent != _right->parent )
		return false; 

	auto parent = _left->parent;

	if( _left->isLeaf() && _right->isLeaf() ){

		if( merge_to_right ){
			// Leaf bpnode - Merge to Right
			for (auto it = _left->pairs.end(); it != _left->pairs.begin(); --it)
			{
				// Change parent node
				_right->pairs.insert(_right->pairs.begin(), *it);
			}
			// if( _left->left_ptr != NULL ) {
			//   _right->becomeRightSibingOf(_left->left_ptr); 
			// }
			_right->becomeRightSibingOf(_left->left_ptr); 

			if( _left->getNextLeaf() != NULL ){
				_left->getNextLeaf()->setNextLeaf(_right);
			}

			_left->forceRemove();

			for(auto it = parent->pairs.begin(); it != parent->pairs.end(); it++)
			{
				if( it->second == _left ){
					parent->pairs.erase(it);
					break;
				}
			}

		}else{
			// Leaf bpnode - Merge to Left

			for (auto it = _right->pairs.begin(); it != _right->pairs.end(); ++it)
			{
				_left->pairs.push_back(*it);
			}

			// if( _right->right_ptr != NULL ){
			//   _right->right_ptr->becomeRightSibingOf(_left);
			// }
			_left->becomeLeftSiblingOf(_right->right_ptr);

			_left->setNextLeaf(_right->getNextLeaf());

			_right->forceRemove();

			itemkey key(0);
			for(auto it = parent->pairs.begin(); it != parent->pairs.end(); it++)
			{
				if( it->second == _right ){
					key = it->first;
					parent->pairs.erase(it);
					break;
				}
			}

			if( parent->getNextLeaf() != _right &&
					key != itemkey(0) ){
				for(auto it = parent->pairs.begin(); it != parent->pairs.end(); it++) {
					if( it->second == _left )
						it->first = key;
				}
			} else {
				parent->pairs.pop_back();
				parent->setNextLeaf(_left);
			}
		}

	}else {

		itemkey key(0); 
		for(auto it = parent->pairs.begin(); it != parent->pairs.end(); it++) {
			if( it->second == _left ){
				key = it->first;
			}
		}

		if( merge_to_right ){
			// Interior bpnode - Merge to Right

			auto new_entry = make_pair( key, _left->getNextLeaf() );

			((bpnode *)new_entry.second)->parent = _right;
			_right->pairs.insert(_right->pairs.begin(), new_entry);

			for (auto it = _left->pairs.end(); it != _left->pairs.begin(); --it)
			{
				// Change parent node
				((bpnode *)it->second)->parent = _right;
				_right->pairs.insert(_right->pairs.begin(), *it);
			}

			// if( _left->left_ptr != NULL ) {
			//   _right->becomeRightSibingOf(_left->left_ptr); 
			// }
			_right->becomeRightSibingOf(_left->left_ptr);

			_left->forceRemove();

			for(auto it = parent->pairs.begin(); it != parent->pairs.end(); it++)
			{
				if( it->second == _left ){
					parent->pairs.erase(it);
					break;
				}
			}
		}else{
			// Interior bpnode - Merge to Left

			auto new_entry = make_pair( key, _left->getNextLeaf());
			_left->pairs.push_back(new_entry);

			for (auto it = _right->pairs.begin(); it != _right->pairs.end(); ++it)
			{
				((bpnode *)it->second)->parent = _left;
				_left->pairs.push_back(*it);
			}

			auto entry = _right->getNextLeaf();
			entry->parent = _left;

			_left->setNextLeaf(entry);

			// if( _right->right_ptr != NULL ){
			//   _right->right_ptr->becomeRightSibingOf(_left);
			// } 
			_left->becomeLeftSiblingOf(_right->right_ptr);

			_right->forceRemove();

			key = -1;
			for(auto it = parent->pairs.begin(); it != parent->pairs.end(); it++) {
				if( it->second == _right ){
					key = it->first;
					parent->pairs.erase(it);
					break;
				}
			}

			if( parent->getNextLeaf() != _right && key != -1 ){
				for(auto it = parent->pairs.begin(); it != parent->pairs.end(); it++) {
					if( it->second == _left )
						it->first = key;
				}
			} else {
				parent->pairs.pop_back();
				parent->setNextLeaf(_left);
			}

		}

	}

	return true;
}







////////////////////////////////////////////////////////////////////////
// bptree class


// Constructor
bptree::bptree(int _key_num) {
	node_number = 0;
	root = new bpnode(this);
	height = 1;
	key_num = _key_num;
}


// Copy constructor
bptree::bptree(const bptree& _copy) {
	(*this) = _copy; // use operator =
}


// Deconstructor
bptree::~bptree() {
}


// Desired assignment overrides
bptree& bptree::operator =(const bptree& _other) {
	root = _other.root;
	height = _other.height;
	key_num = _other.key_num;
	node_number = _other.node_number;
	return *this;
}


bool bptree::insert(itemkey  _key, itemptr& _value) {

	bpnode * current_node = root;
	while ( !current_node->isLeaf() ) {
		current_node = (bpnode *)current_node->findChild(_key);
	}

	itemkey key = _key;
	bpentry * entry = new bpvalue(&_value);
	int insert_status;
	while ( (insert_status = current_node->insert(key, entry)) == FULL ) {
		auto p = current_node->split(key, entry);
		if (p.second == NULL) break;
		key = p.first;
		entry = p.second;
		if (current_node->parent == NULL) {
			makeNewRoot(key, (bpnode *)entry, current_node);
			insert_status = SUCCESS;
			break;
		}
		else {
			current_node = current_node->parent;
		}
	}

	return ( insert_status == SUCCESS );
}


bool bptree::remove(itemkey _key) {

	bpnode * current_node = root;
	while( !current_node->isLeaf() ){
		current_node = (bpnode *) current_node->findChild(_key);
	}

	int delete_status;

	while( (delete_status = current_node->remove(_key)) == TOO_FEW_KEYS )
	{


#ifdef DEBUG
		cout << "bptree::remove - Current bpnode has too few keys..." << endl;
#endif

		if( current_node->left_ptr != NULL                        && 
				bpnode::isSibling(current_node->left_ptr, current_node) && 
				current_node->left_ptr->hasExtraKeys()                ){

#ifdef DEBUG
			cout << "bptree::remove - Redistribute: from left sibling to right node..." << endl;
#endif

			// 1. Redistribute to left sibling
			bpnode::redistribute( current_node->left_ptr, current_node );

		}else if( current_node->right_ptr != NULL                        &&
				bpnode::isSibling(current_node, current_node->right_ptr) && 
				current_node->right_ptr->hasExtraKeys()                ){
			// 2. Redistribute to right sibling


#ifdef DEBUG
			cout << "bptree::remove - Redistribute: from right sibling to left node..." << endl;
#endif


			bpnode::redistribute( current_node, current_node->right_ptr, true );

		}else if( current_node->left_ptr != NULL                          && 
				bpnode::isSibling(current_node->left_ptr, current_node)   && 
				!current_node->left_ptr->hasExtraKeys()                 ){
			// 3. Coalesce left sibling

#ifdef DEBUG
			cout << "bptree::remove - Coalesce: left sibling and current node" << endl;
#endif


			bpnode::coalesce(current_node->left_ptr, current_node);

		}else if( current_node->right_ptr != NULL                        &&
				bpnode::isSibling(current_node, current_node->right_ptr) && 
				!current_node->right_ptr->hasExtraKeys()               ){
			// 4. Coalesce right sibling


#ifdef DEBUG
			cout << "bptree::remove - Coalesce: current node and right sibling" << endl;
#endif

			bpnode::coalesce(current_node, current_node->right_ptr);
		}
		else {
			// 5. Single node case

#ifdef DEBUG
			if( isRoot(current_node) ){
				cout << "bptree::remove - Single root case: normal" << endl;
			}else{
				cout << "Abnormal Case..." << endl;
			}
			cout << "bptree::remove - Single bpnode Case" << endl;
#endif      
		}

		if ( current_node->parent != NULL ) {
			current_node = current_node->parent;

#ifdef DEBUG
			cout << "bptree::remove - Iterate parent node" << endl;
#endif  

		}
		else
		{

#ifdef DEBUG
			cout << "bptree::remove - Reach the root of tree" << endl;
			cout << "bptree::remove - Cut down tree height" << endl;
#endif       
			// Delete empty root node and make the second level nodes the be the root
			deleteEmptybpnode(current_node);
			break;

		}

	}

	if( delete_status == KEY_NOT_FOUND) 
		// The case that there is no such key in the bpTree
		return false;

	return ( delete_status == SUCCESS );
}


itemptr * bptree::find(itemkey _key) {
	bpnode * current_node = root;
	while ( !current_node->isLeaf() ) {
		current_node = (bpnode *)current_node->findChild(_key);
	}
	bpentry * valuebpentry = current_node->findValueEntry(_key);
	if (valuebpentry == NULL) {
		return NULL;
	}
	else {
		return ((bpvalue *)valuebpentry)->getvalue();
	}
}


void bptree::printKeys() {
	queue< pair<int, bpnode *> > que;
	que.push( make_pair(1, root) ); 
	int cur_level = 1;
	while ( !que.empty() ) {
		auto cur = que.front();
		que.pop();
		if ( !cur.second->isLeaf() ) {
			for (auto it = cur.second->pairs.begin(); it != cur.second->pairs.end(); it++) { 
				que.push( make_pair(cur.first + 1, (bpnode *)it->second) );
			}
			que.push( make_pair(cur.first + 1, (bpnode *)cur.second->extra_entry) );
		}
		if (cur.first > cur_level) {
			printf("\n");
			cur_level = cur.first;
		}
		cur.second->printKeys();
	}

	printf("\n");
}


void bptree::printValues() {
	// Find leftmost key and print out values for the bpTree
	bpnode * current_node = root;
	while ( !current_node->isLeaf() ) {
		current_node = (bpnode *)current_node->findLeftMostChild();
	}

	// Iterate the leaf level tree nodes
	do{
		current_node->printValues();
		current_node = current_node->extra_entry;
	}
	while ( current_node );

	printf("\n");  
}

