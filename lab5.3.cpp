#include <stdio.h>
#define E 0

struct suffix_tree_node;


struct suffix_tree_link {
	// 0 is e - global index of during string's end
	unsigned long start;
	unsigned long end;
	suffix_tree_link(suffix_tree_node* source, suffix_tree_node* target, 
					 unsigned long start, unsigned long end) {
		this->source = source;
		this->target = target;
		this->start = start;
		this->end = end;
	}
	suffix_tree_node* source;
	suffix_tree_node* target;
	suffix_tree_link* next_link;
};

struct suffix_tree_node {
	suffix_tree_link* first_link;
	suffix_tree_node* parent_node;
	suffix_tree_node* suffix_link_node;
	
	// other constructors?
	suffix_tree_node() {
		parent_node = suffix_link_node = NULL;
		first_link = NULL;
	}
	
	void add_target(unsigned long start, unsigned long end, suffix_tree_node* target) {
		suffix_tree_link* link;
		for(link = first_link; link != NULL; link = link->next_link);
		link = new suffix_tree_link(this, target, start, end);
	}
	
};



class suffix_tree {

	suffix_tree_node* root;
	const char* string;
	
	void ukkonen() {
		root->add_target(1, E, new suffix_tree_node);
		unsigned long e = 1, j_i = 1, i = 1; 
		for( int i = 0; string[i] != '\0'; i++) {
			e++;
			for() {
				
				j_star = j;
			}
			
			j_i = j_star;
		}
	}
	
public:

	suffix_tree(const char* string) {
		root = new suffix_tree_node();
		this->string = string;
		ukkonen();
	}

};

int main() {
	suffix_tree("foof");
	return 0;
}
