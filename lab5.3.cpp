#include <stdio.h>

struct suffix_tree_link;

struct suffix_tree_node {
	suffix_tree_link first_link;

	suffix_tree_node() {
		first_link = NULL;
	}
};

struct suffix_tree_link {
	unsigned long start;
	unsigned long end;
	suffix_tree_link(unsigned long start, unsigned long end) {
		this->start = start;
		this->end = end;
	}
	state* target;
	suffix_tree_link* next_link;
};

class suffix_tree {

	suffix_tree_node root;

public:

	suffix_tree() {
		root = new suffix_tree_node();
	}

};

int main() {
	return 0;
}
