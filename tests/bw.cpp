
#include "bptree.h"
#include <vector>
#include <iostream>

using namespace std;

int main() {
	bptree<key128_t, long, rquery<key128_t>> tree;
	vector<long> values {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	for (auto item : values) {
		tree.insert(key128_t(item), item);
	}
	auto iter = tree.find(key128_t(7));
	cout << iter->second << endl;
}



