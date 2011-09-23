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
	unsigned long e;

	void ukkonen() {

		root->add_target(1, E, new suffix_tree_node(1, root, NULL));
		e = 1;
		unsigned long j_i = 1;

		// expand suffix tree with string[i + 1] char
		for( unsigned long i = 1; string[i] != '\0'; i++ ) {
			e++;
			suffix_tree_node* node = root;

			unsigned long jump = 0;
//			bool first = true;

			// node to make suffix link for it
			suffix_tree_node* prev_last = NULL;

			unsigned long j_star; // last j on previous phase

			for(unsigned long j = j_i + 1; j <= i + 1; j++) {

//				unsigned long length;
				printf("try to insert S[%lu %lu] with jump %lu\n", j, i + 1, jump);
				suffix_tree_node* last_node; // last node on the searched path
				suffix_tree_node* parent_node; // parent of new fork or == last_node if no fork was created

				bool result = insert(node, j, i, parent_node, last_node);

				if(last_node != parent_node)
					printf("fork parent: %lu last: %lu\n", parent_node->label, last_node->label);

				if( result == false ) { // 3rd rule => break phase
					if(prev_last != NULL) {
						printf("creating suffix link from %lu to %lu\n", prev_last->label,last_node->label);
						prev_last->suffix_link_node = last_node; // suffix link to last node on the searched path
					}
					break;
				}

				printf("parent: %lu suffix_link: %lu\n", parent_node->label, parent_node->suffix_link_node->label);
				if( parent_node->suffix_link_node != NULL ) {
					printf("go from %lu to %lu by suffix link\n", parent_node->label, parent_node->suffix_link_node->label);
					node = parent_node->suffix_link_node;
				} else {
					jump = 0;
//					node = root; // not needed
				}

				if(prev_last != NULL) {
					printf("creating suffix link from %lu to %lu\n", prev_last->label, last_node->label);
					prev_last->suffix_link_node = last_node; // suffix link to last node on path to new node
				}

				prev_last = last_node;

				j_star = j;
			}

			j_i = j_star;
		}
	}

	// returns 'true' if node was created and last_node - it's parent
	// or 'false', if this path exists already and last_node - last node on path to it
	// parent_node - parent of new fork or == last_node, if no fork was created
	bool insert( suffix_tree_node* node, unsigned long j, unsigned long i, suffix_tree_node*& parent_node, suffix_tree_node*& last_node ) {

		printf("inserting path: ");
		for(unsigned long n = j; n <= i + 1; n++) {
			printf("%c ", string[n - 1]);
		}
		printf("\n");

		for(;;) {

			printf("in node %lu\n", node->label);

			unsigned long j1, j2; // char counters for string and link
			suffix_tree_link* link;
			for(link = node->first_link; link != NULL; link = link->next_link) {

				printf("compare: %c * %c\n", string[j - 1], string[link->start - 1] );

				if( string[j - 1] == string[link->start - 1] ) {
					j1 = j;
					j2 = link->start;
					printf("go to %c\n", string[j - 1]);
					break;
				}
			}

			// add new link and node directly to node
			if( link == NULL ) {
				printf("adding directly to node: %lu - E \n", i + 1);
				node->add_target(i + 1, E, new suffix_tree_node(j, node, NULL));
				parent_node = last_node = node;
				return true;
			}

			bool go = false;
			do {
				if ( j1 == i + 1 ) { // string in tree already
					printf("string in tree already\n");
					// do nothing
					parent_node = last_node = node;
					return false;
				}
				if ( j2 == link->end ) { // link is over
					printf("go to next node\n");
					node = link->target; // go to next node
					go = true;
					break;
				}
				j1++; j2++;
				printf("compare link's chars: %c * %c\n", string[j1 - 1], string[j2 - 1]);
			} while (string[j1 - 1] == string[j2 - 1]);

			if( go ) { // go to target node
//				length += ( link->end - link->start + 1);
				j = j1 + 1;
				continue;
			}

			// split old link
			suffix_tree_node* next = link->target;
			unsigned long old_end = link->end;
			printf("split link %lu - %lu to %lu - %lu and %lu - %lu\n",
					link->start, link->end,
					link->start, j2 - 1, j2, link->end);
			link->end = j2 - 1; link->target = new suffix_tree_node(j, node, NULL);
			link->target->add_target(j2, old_end, next);

			// add new link with new node
			printf("adding link to separator node: %lu - E \n", j1);
			link->target->add_target(j1, E, new suffix_tree_node(j, link->target, NULL));

			// return parent of new node
			parent_node = node;
			last_node = link->target;
			return true;
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
		printf("e = %lu\n", e);
	}

};

int main() {
//	suffix_tree st("xabdabexabc");
//	suffix_tree st("axabaxaxaaxabx");
//	suffix_tree st("axabaxaxaax");
//	suffix_tree st("axabaxax");
//	suffix_tree st("");
//	suffix_tree st("abc");
//	suffix_tree st("abaab$");
	suffix_tree st("mama$");
	st.print();
	return 0;
}

