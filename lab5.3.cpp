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
		this->next_link = NULL;
	}
	suffix_tree_node* source;
	suffix_tree_node* target;
	suffix_tree_link* next_link;
};

struct suffix_tree_node {
	suffix_tree_link* first_link;
	suffix_tree_node* parent_node; // parent node is needed?
	suffix_tree_node* suffix_link_node;
	unsigned long label;
	
	// constructor for root node
	suffix_tree_node(unsigned long label) {
		parent_node = NULL;
		suffix_link_node = this;
		first_link = NULL;
		this->label = label;
	}
	
	// constructor for leaf node
	suffix_tree_node(unsigned long label, suffix_tree_node* parent_node, suffix_tree_node* suffix_link_node) {
		this->parent_node = parent_node;
		this->suffix_link_node = suffix_link_node;
		first_link = NULL;
		this->label = label;
	}

	void add_target(unsigned long start, unsigned long end, suffix_tree_node* target) {
//		printf("adding node...\n");
		suffix_tree_link** link;
		for(link = &first_link; *link != NULL; link = &(*link)->next_link);

		*link = new suffix_tree_link(this, target, start, end);
	}
	
};



class suffix_tree {

	suffix_tree_node* root;
	const char* string;

	void ukkonen() {

		root->add_target(1, E, new suffix_tree_node(1, root, NULL));
		unsigned long e = 1, j_i = 1;
		// expand suffix tree with string[i + 1] char
		for( unsigned long i = 1; string[i] != '\0'; i++ ) {
			e++;
			suffix_tree_node* node = root;

			unsigned long jump = 0;
			bool first = true;

			// node to make suffix link for this
			suffix_tree_node* prev_parent = NULL;

			unsigned long j_star; // last j on previous phase

			for(unsigned long j = j_i + 1; j <= i + 1; j++) {

				unsigned long length;
				printf("try to insert S[%lu %lu] with jump %lu\n", j, i + 1, jump);
				suffix_tree_node* parent = insert(node, j, i, length);

				if( parent == NULL ) // 3rd rule => break phase
					break;

				if( parent->suffix_link_node != NULL ) {
					if( first ) {
						jump = length - 1;
						first = false;
					} else
						jump--;
					node = node->suffix_link_node;
				} else {
					jump = 0;
					node = root; // not needed
				}

				if(prev_parent != NULL)
					prev_parent->suffix_link_node = parent;

				prev_parent = parent;

				j_star = j;
			}

			j_i = j_star;
		}
	}

	// returns new node's parent or NULL, if this path exists already
	// length is count of passed symbols before new node's parent (needed to pass symbols after jumping by suffix link)
	suffix_tree_node* insert(suffix_tree_node* node, unsigned long j, unsigned long i, unsigned long& length) {

		length = 0;

		if( j == 8 && i + 1 == 10) {
			printf("%c %c %c\n", string[7], string[8], string[9]);
		}

		for(;;) {

			if( j == 8 && i + 1 == 10) {
				printf("start\n");
			}

			unsigned long j1, j2; // char counters for string and link
			suffix_tree_link* link;
			for(link = node->first_link; link != NULL; link = link->next_link) {
				if( j == 8 && i + 1 == 10) {
					printf("%c * %c\n", string[j - 1], string[link->start - 1] );
				}
				if( string[j - 1] == string[link->start - 1] ) {
					j1 = j;
					j2 = link->start;
					break;
				}
			}

			// add new link and node directly to node
			if( link == NULL ) {

				if( j == 8 && i + 1 == 10) {

					printf("adding link: %lu - e \n", i + 1);
				}

				node->add_target(i + 1, E, new suffix_tree_node(j, node, NULL));
				return node;
			}

			bool go = false;
			do {
				if ( j1 == i + 1 ) { // string in tree already ERROR!?
					if( j == 8 && i + 1 == 10) {
						printf("string in tree already\n");
					}
					return NULL; // do nothing
				}
				if ( j2 == link->end ) { // link is over
					node = link->target; // go to next node
					go = true;
					break;
				}
				j1++; j2++;
			} while (string[j1 - 1] == string[j2 - 1]);

			if( go ) { // go to target node
				length += ( link->end - link->start + 1);
				j = j1;
				continue;
			}

			// split old link
			suffix_tree_node* next = link->target;
			unsigned long old_end = link->end;
			link->end = j2 - 1; link->target = new suffix_tree_node(j, node, NULL);
			link->target->add_target(j2, old_end, next);

			// add new link with new node
//			printf("adding link: %lu - e \n", j1);
			link->target->add_target(j1, E, new suffix_tree_node(j, link->target, NULL));

			// return parent of new node
			return link->target;
		}
	}

void print_spaces(int n) {
	for(int i = 0; i < n; i++) {
		printf(" ");
	}
}

void print_node(suffix_tree_node* node, int deep) {
	print_spaces(deep);
	printf("node: %lu\n", node->label);
	for(suffix_tree_link* link = node->first_link; link != NULL; link = link->next_link) {
		print_spaces(deep);
		printf("start: %lu end: %lu\n", link->start, link->end);
		print_node(link->target, deep + 1);
	}
}

public:

	void print() {
		print_node(root, 0);
	}

	suffix_tree(const char* string) {
		this->string = string;
		root = new suffix_tree_node(0);
		ukkonen();
	}

};

int main() {
//	suffix_tree st("xabdabexabc");
//	suffix_tree st("axabaxaxaaxabx");
	suffix_tree st("axabaxaxaax");
	st.print();
	return 0;
}

