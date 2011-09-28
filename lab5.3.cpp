#include <stdio.h>
#include <cstring>

#define E 0
#define MIN(X,Y) (((X) < (Y)) ? (X) : (Y))

// compares two string and returns length of equal prefixes
int compare(const char* str1, const char* str2, unsigned long& length) {
	unsigned long i = 0;
	while( str1[i] == str2[i] ) {
		if( str1[i] == '\0' ) {
			length = i;
			return 0; // equal
		}
		i++;
	}
	length = i;
	if( str1[i] > str2[i] ) {
		return 1;
	} else {
		return -1;
	}
}

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
		// printf("adding node...\n");
		suffix_tree_link** link;
		for(link = &first_link; *link != NULL; link = &(*link)->next_link);

		*link = new suffix_tree_link(this, target, start, end);
	}

	// instert new link to node in alphabetical order
	// TODO: optimise search
	void add_target(unsigned long start, unsigned long end, suffix_tree_node* target, const char* string) {
//		printf("adding node...\n");
		suffix_tree_link** link;
		for(link = &first_link; *link != NULL; link = &(*link)->next_link) {
			if(string[start - 1] <= string[(*link)->start - 1])
				break;
		}

		// inserting new link before old
		suffix_tree_link* next = *link;
		*link = new suffix_tree_link(this, target, start, end);
		(*link)->next_link = next;
	}
	
};

class suffix_tree {

	suffix_tree_node* root;
	const char* string;
	unsigned long e;

	void ukkonen() {

		root->add_target(1, E, new suffix_tree_node(1, root, NULL), string);
		e = 1;
		unsigned long j_i = 1;

		// expand suffix tree with string[i + 1] char
		for( unsigned long i = 1; string[i] != '\0'; i++ ) {
			e++;
			suffix_tree_node* node = root;

			unsigned long distance = 0; // distance from root using to calculate skip after transition by suffix link
			unsigned long skip = 0; // how many chars must be skipped after suffix transition
//			bool first = true;

			// node to make suffix link for it
			suffix_tree_node* prev_last = NULL;

			unsigned long j_star; // last j on previous phase
//			bool first = true;

			for(unsigned long j = j_i + 1; j <= i + 1; j++) {


				printf("try to insert S[%lu %lu]\n", j + skip, i + 1);
				suffix_tree_node* last_node; // last node on the searched path
				suffix_tree_node* parent_node; // parent of new fork or == last_node if no fork was created

				unsigned long length;

				bool result = insert(node, j, skip, i, parent_node, last_node, length);
				printf("length = %lu\n", length);

				distance += length;

				printf("distance to last node: %lu\n", distance);

				skip = (distance > 1) ? distance - 1 : 0;

				if(j != i + 1)
					printf("next skip = %lu\n", skip);

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
					distance = (distance != 0) ? distance - 1 : 0;
				} else {
					distance = 0;
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
	bool insert( suffix_tree_node* node, unsigned long j, unsigned long skip, unsigned long i,
			suffix_tree_node*& parent_node, suffix_tree_node*& last_node, unsigned long& distance ) {

		printf("leaf label: %lu\n", j);
		unsigned long leaf_label = j;

		j += skip;

		printf("j after skip: %lu\n", j);

		// distance to last node
		distance = 0;

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
				node->add_target(i + 1, E, new suffix_tree_node(leaf_label, node, NULL), string);
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
				distance += ( link->end - link->start + 1 );
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
			link->target->add_target(j2, old_end, next, string);

			// add new link with new node
			printf("adding link to separator node: %lu - E \n", j1);
			link->target->add_target(j1, E, new suffix_tree_node(leaf_label, link->target, NULL), string);

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
	printf("node: %lu", node->label);
	if(node->first_link == NULL)
		printf("*\n");
	else
		printf("\n");
	for(suffix_tree_link* link = node->first_link; link != NULL; link = link->next_link) {
		print_spaces(deep);
		printf("start: %lu [%c] end: %lu\n", link->start, string[link->start - 1], link->end);
		print_node(link->target, deep + 1);
	}
}

unsigned long* array;
unsigned long* lcp_arr, * lcp_arr_pt;
unsigned long index;

void walk(suffix_tree_node* node, unsigned long depth) {
	for(suffix_tree_link* link = node->first_link; link != NULL; link = link->next_link) {
		if(link->end != E) {
			walk(link->target, depth + link->end - link->start + 1 );
		}
		else {
			array[index] = link->target->label;
			//printf("%lu ", link->target->label);
			index++;
		}
		if (link->next_link != NULL) {
			printf("%lu ", depth);
			*lcp_arr_pt = depth;
			lcp_arr_pt++;
		}
	}
}

public:

	void print() {
		print_node(root, 0);
	}

	unsigned long* get_suffix_array() {
		printf("suffix array length %lu: \n", e);
		array = new unsigned long[e];
		index = 0;
		lcp_arr = lcp_arr_pt = new unsigned long [e - 1];
		printf("walk depths:\n");
		walk(root, 0);
		printf("\n");
		return array;
	}

	unsigned long* get_lcp_arr() {
		return lcp_arr;
	}

	unsigned long get_size() {
		return e;
	}

	const char* get_string() {
		return string;
	}

	suffix_tree(const char* string) {
		this->string = string;
		root = new suffix_tree_node(0);
		ukkonen();
//		printf("e = %lu\n", e);
	}

};

struct lcp_tree_node {

	unsigned long value;

	lcp_tree_node* left_node;
	lcp_tree_node* right_node;


	lcp_tree_node(unsigned long left, unsigned long right, unsigned long* lcp_arr) {
		if(right != left + 1) {
			unsigned long middle = (left + right) / 2; // TODO: fix
			left_node = new lcp_tree_node(left, middle, lcp_arr);
			right_node = new lcp_tree_node(middle, right, lcp_arr);
			value = MIN( left_node->value, right_node->value );
		} else {
			value = lcp_arr[left];
		}
		printf("interval [%lu %lu] value = %lu\n", left, right, value);
	}

};

struct lcp_tree {

	unsigned long* lcp_arr; // array of lcp between neigbour suffixes in suffix array

	lcp_tree_node* root;

public:

	lcp_tree(suffix_tree& st) {
		lcp_arr = st.get_lcp_arr();
		printf("lcp_arr\n");
		for (unsigned long i = 0; i < st.get_size() - 1; i++) {
			printf("%lu ", lcp_arr[i]);
		}
		printf("\n");
		unsigned long L = 1, R = st.get_size() - 1;
		root = new lcp_tree_node(L, R, lcp_arr);
	}

};

class suffix_array {

	const char* string;
	unsigned long* pos;
	unsigned long size;
	lcp_tree* lt;
	lcp_tree_node* lcp_node;

	// returns count of equal chars at start of pattern and suffix string[pos .. size - 1]
	unsigned long match_prefix_length(const char* pattern, unsigned long n) {

		unsigned long j = 0;
		printf("%s\n", string);
		printf("%s\n", string + n - 1);
		printf("n = %lu\n", n);
		for(unsigned long i = n - 1; i < size; i++) {
			printf("comparing %c * %c\n", pattern[j], string[i]);
			if ( pattern[j] != string[i] ) {
				break;
			}
			j++;
		}
		printf("returned value: %lu\n", j);
		return j;
	}

public:

	suffix_array(suffix_tree& st) {
		string = st.get_string();
		pos = st.get_suffix_array();
		size = st.get_size();

		// tree to find lcp(i, j) values fast
		lcp_tree lt(st);
	}

	// prints suffixies according numbers in array
	void print(FILE* file) {
		for(unsigned long i = 0; i < size; i++) {
			for(unsigned long j = pos[i]; j < size + 1; j++) {
				fprintf(file, "%c", string[j - 1]);
			}
			fprintf(file, "\n");
		}
	}

	// TODO: make private
	bool decision( const char* pattern, unsigned long M, unsigned long& L, unsigned long& R, unsigned long& l, unsigned long& r ) {
		unsigned long lcp = lcp_node->value;
		if( lcp > l) {
			L = M;
		} else if ( lcp < l ) {
			R = M;
			r = lcp;
		} else { // lcp == l
			unsigned long length;
			int result = compare(pattern + l + 1, string + l + 1 + pos[M], length);
			if ( result < 0 ) {
				R = M;
				lcp_node = lcp_node->left_node; // go left child
				r = lcp + length;
			} else if ( result > 0 ) {
				L = M;
				lcp_node = lcp_node->right_node; // go right child
				l = lcp + length;
			} else {
				return true; // found
			}
		}
		return false;
	}

	unsigned long search(const char* pattern) {

		lcp_node = lt->root; // set root lcp value

		unsigned long L = 1, R = size - 1; // pos[0] is "$"
		printf("suze = %lu\n", size);

//		printf("%lu\n", match_prefix_length("aabcab", 2)); // test

		printf("pos[L] = %lu pos[R] = %lu\n", pos[L], pos[R]);

		unsigned long l = match_prefix_length(pattern, pos[L]);
		unsigned long r = match_prefix_length(pattern, pos[R]);
		unsigned long M;

		for(;;) {

			M = (L + R) / 2; //TODO: correct this by Bloch article

			if(l == r) { // == lcp
				unsigned long length;
				int result = compare(pattern + l + 1, string + l + 1 + pos[M], length);
				if ( result < 0 ) {
					R = M;
					lcp_node = lcp_node->left_node; // go left child
					r += length;
				} else if ( result > 0 ) {
					L = M; // lcp == l
					lcp_node = lcp_node->right_node; // go to right child
					l += length;
				} else {
					break; // found
					printf("!found at %lu ", l);
				}
				continue;
			}

			bool result = (l > r) ? decision(pattern, M, L, R, l, r) : decision(pattern, M, R, L, r, l);

//			if(l > r) {
//				result = decision(pattern, M, L, R, l, r);
//			} else {
//				result = decision(pattern, M, R, L, r, l);
//			}

			if(result == true) {
				printf("found at %lu ", M);
				break;
			} else {
				continue;
			}
		}
		return 0;
	}

};

int main() {

	const char* str = "tartar$";

	suffix_tree st(str);
	st.print();

	FILE* file = fopen("sarr.txt", "w");

	suffix_array sa(st);
	sa.print(file);
	sa.search("ta");
	fclose(file);


//	unsigned long len;
//	int s = compare("a", "a", len);
//	printf("%d %lu", s, len);

	return 0;
}

