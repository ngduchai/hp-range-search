
#include "bptree.h"
#include <vector>
#include <iostream>

using namespace std;

int main() {
	bptree<long, long, rquery<long>> tree;
	vector<long> values {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	for (auto item : values) {
		tree.insert(item, item);
	}
	auto iter = tree.find(rquery<long>(6, 7));
	while (iter.valid()) {
		cout << iter.value() << endl;
		iter.next();
	}
}



