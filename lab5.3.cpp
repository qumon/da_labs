#include <stdio.h>
#include <cstring>
#include <cstdlib>

#define E 0
#define MIN(X,Y) (((X) < (Y)) ? (X) : (Y))

// return 0 if str2 starts with str1, length is count of equal chars at start
int starts_with(const char* str1, const char* str2, unsigned long& length ) {
	unsigned long i = 0;
	for(;;) {
		if( str1[i] == '\0' ) {
			length = i;
			return 0; // prefix
		}
//		printf("comparing chars %c %c\n", str1[i], str2[i]);
		if ( str1[i] != str2[i] )
			break;
		i++;
	};
	length = i;
	if( str1[i] > str2[i] ) {
		return 1;
	} else {
		return -1;
	}
}

// return true if str2 starts with str1, comparing starts from skip - 1 chars
bool starts(const char* str1, const char* str2, unsigned long skip) {
//	printf("comparing %s & %s skip = %lu\n", str1, str2, skip);
	unsigned long n = skip - 1;
	for(;;) {
//		printf("n = %lu ", n);
//		printf("char cmpr c1=%c c2=%c\n", str1[n], str2[n]);
		if(str1[n] != str2[n]) {
//			printf("false\n");
			return false;
		}
		if(n == 0)
			break;
		n--;
	}
//	printf("true\n");
	return true;
}

template <class number>
void swap(number& el1, number& el2) {
    number s = el1;
    el1 = el2;
    el2 = s;
}

template <class number>
unsigned long partition(number* section, unsigned long start, unsigned long finish, bool (*compare)(number, number) )
{
 unsigned long pivot = start;
 number value = section[pivot];
   for( unsigned long index = start + 1; index < finish ; ++index )
     if( !compare( section[index], value ) )
       swap(section[index], section[++pivot]);
  swap(section[start], section[pivot]);
 return pivot;
}

template <class number>
void quicksort(number* section, unsigned long start, unsigned long finish, bool (*compare)(number, number) )
{
    if(start < finish)
  {
   unsigned long pivot = partition(section, start, finish, compare);
   quicksort(section, start, pivot, compare);
   quicksort(section, pivot + 1, finish, compare);
  }
 return;
}

template <class number>
void quicksort(number* section, unsigned long length, bool (*compare)(number, number) )
{
 quicksort(section, 0, length, compare);
}

bool compare(unsigned long el1, unsigned long el2) {
    if(el1 >= el2)
        return true;
    else
        return false;
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


//				printf("try to insert S[%lu %lu]\n", j + skip, i + 1);
				suffix_tree_node* last_node; // last node on the searched path
				suffix_tree_node* parent_node; // parent of new fork or == last_node if no fork was created

				unsigned long length;

				bool result = insert(node, j, skip, i, parent_node, last_node, length);
//				printf("length = %lu\n", length);

				distance += length;

//				printf("distance to last node: %lu\n", distance);

				skip = (distance > 1) ? distance - 1 : 0;

//				if(j != i + 1)
//					printf("next skip = %lu\n", skip);

//				if(last_node != parent_node)
//					printf("fork parent: %lu last: %lu\n", parent_node->label, last_node->label);

				if( result == false ) { // 3rd rule => break phase
					if(prev_last != NULL) {
//						printf("creating suffix link from %lu to %lu\n", prev_last->label,last_node->label);
						prev_last->suffix_link_node = last_node; // suffix link to last node on the searched path
					}
					break;
				}

//				printf("parent: %lu suffix_link: %lu\n", parent_node->label, parent_node->suffix_link_node->label);
				if( parent_node->suffix_link_node != NULL ) {
//					printf("go from %lu to %lu by suffix link\n", parent_node->label, parent_node->suffix_link_node->label);
					node = parent_node->suffix_link_node;
					distance = (distance != 0) ? distance - 1 : 0;
				} else {
					distance = 0;
//					node = root; // not needed
				}

				if(prev_last != NULL) {
//					printf("creating suffix link from %lu to %lu\n", prev_last->label, last_node->label);
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

//		printf("leaf label: %lu\n", j);
		unsigned long leaf_label = j;

		j += skip;

//		printf("j after skip: %lu\n", j);

		// distance to last node
		distance = 0;

//		printf("inserting path: ");
//		for(unsigned long n = j; n <= i + 1; n++) {
//			printf("%c ", string[n - 1]);
//		}
//		printf("\n");

		for(;;) {

//			printf("in node %lu\n", node->label);

			unsigned long j1, j2; // char counters for string and link
			suffix_tree_link* link;
			for(link = node->first_link; link != NULL; link = link->next_link) {

//				printf("compare: %c * %c\n", string[j - 1], string[link->start - 1] );

				if( string[j - 1] == string[link->start - 1] ) {
					j1 = j;
					j2 = link->start;
//					printf("go to %c\n", string[j - 1]);
					break;
				}
			}

			// add new link and node directly to node
			if( link == NULL ) {
//				printf("adding directly to node: %lu - E \n", i + 1);
				node->add_target(i + 1, E, new suffix_tree_node(leaf_label, node, NULL), string);
				parent_node = last_node = node;
				return true;
			}

			bool go = false;
			do {
				if ( j1 == i + 1 ) { // string in tree already
//					printf("string in tree already\n");
					// do nothing
					parent_node = last_node = node;
					return false;
				}
				if ( j2 == link->end ) { // link is over
//					printf("go to next node\n");
					node = link->target; // go to next node
					go = true;
					break;
				}
				j1++; j2++;
//				printf("compare link's chars: %c * %c\n", string[j1 - 1], string[j2 - 1]);
			} while (string[j1 - 1] == string[j2 - 1]);

			if( go ) { // go to target node
				distance += ( link->end - link->start + 1 );
				j = j1 + 1;
				continue;
			}

			// split old link
			suffix_tree_node* next = link->target;
			unsigned long old_end = link->end;
//			printf("split link %lu - %lu to %lu - %lu and %lu - %lu\n",
//					link->start, link->end,
//					link->start, j2 - 1, j2, link->end);
			link->end = j2 - 1; link->target = new suffix_tree_node(j, node, NULL);
			link->target->add_target(j2, old_end, next, string);

			// add new link with new node
//			printf("adding link to separator node: %lu - E \n", j1);
			link->target->add_target(j1, E, new suffix_tree_node(leaf_label, link->target, NULL), string);

			// return parent of new node
			parent_node = node;
			last_node = link->target;
			return true;
		}
	}


// for debug
void print_spaces(int n) {
	for(int i = 0; i < n; i++) {
		printf(" ");
	}
}

// for debug
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
//			printf("%lu ", depth);
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
//		printf("suffix array length %lu: \n", e);
		array = new unsigned long[e];
		index = 0;
		lcp_arr = lcp_arr_pt = new unsigned long [e - 1];
//		printf("walk depths:\n");
		walk(root, 0);
//		printf("\n");
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

		if(left == right && left == 1) {
			value = lcp_arr[0];
		} else if( right != left + 1 ) {
			unsigned long middle = (left + right) / 2; // TODO: fix
			left_node = new lcp_tree_node(left, middle, lcp_arr);
			right_node = new lcp_tree_node(middle, right, lcp_arr);
			value = MIN( left_node->value, right_node->value );
		} else {
			left_node = right_node = NULL;
			value = lcp_arr[left];
		}
//		printf("interval [%lu %lu] value = %lu\n", left, right, value);
	}

};

struct lcp_tree {

	unsigned long* lcp_arr; // array of lcp between neigbour suffixes in suffix array

	lcp_tree_node* root;

public:

	lcp_tree(suffix_tree& st) {
		lcp_arr = st.get_lcp_arr();
//		printf("lcp_arr\n");
//		for (unsigned long i = 0; i < st.get_size() - 1; i++) {
//			printf("%lu ", lcp_arr[i]);
//		}
//		printf("\n");
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
//		printf("%s\n", string);
//		printf("%s\n", string + n - 1);
//		printf("n = %lu\n", n);
		for(unsigned long i = n - 1; i < size; i++) {
			//printf("comparing %c * %c\n", pattern[j], string[i]);
			if ( pattern[j] != string[i] ) {
				break;
			}
			j++;
		}
//		printf("returned value: %lu\n", j);
		return j;
	}
	// find left and rigth borders of matching interval of pos array (based on founded value M)
	void expand( const char* pattern, unsigned long M, unsigned long skip ) {
		unsigned long start, end;
		unsigned long n = M - 1;
		while (starts(pattern, string + pos[n] - 1, skip) && n > 0) {
			n--;
		}
		n++;
		start = n;
//		while(n <= M) {
//			printf("%lu -", pos[n]);
//			n++;
//		}
		n = M;
		while (starts(pattern, string + pos[n] - 1, skip) && n < size) {
			n++;
		}
		end = n - 1;
//		printf("start %lu, end %lu", start, end);

//		printf("\n");
		// output unsorted array:
//		printf("unsorted array\n");
//		for(unsigned long i = start; i <= end; i++) {
//			printf("%lu ", pos[i]);
//		}
//		printf("\n");
		// copy suffix array interval
		unsigned long size = end - start + 1;
		unsigned long* result = new unsigned long[ size ];
		memcpy(result, pos + start, size * sizeof(unsigned long));
		quicksort(result, size, compare);

		// RESULT OUTPUT
		for(unsigned long i = 0; i < size; i++) {
			printf("%lu", result[i]);
			if(i != size - 1)
				printf(", ");
		}
		delete[] result;
	}

public:

	suffix_array(suffix_tree& st) {
		string = st.get_string();
		pos = st.get_suffix_array();
		size = st.get_size();

		// tree to find lcp(i, j) values fast
		lt = new lcp_tree(st);
	}

	// debug method
	// prints suffixies according numbers in array
	void print(FILE* file) {
		for(unsigned long i = 0; i < size; i++) {
			fprintf(file, "%lu ", i);
			for(unsigned long j = pos[i]; j < size + 1; j++) {
				fprintf(file, "%c", string[j - 1]);
			}
			fprintf(file, "\n");
		}
	}

	int search_new(const char* pattern) {
//		printf("starting search...\n");

		lcp_node = lt->root; // set root lcp value

		unsigned long L = 1, R = size - 1; // pos[0] is "$"

		unsigned long l = match_prefix_length(pattern, pos[L]);
		unsigned long r = match_prefix_length(pattern, pos[R]);

		unsigned long M;

		int iter = 0;

		for(;;) {

			iter++;

//			printf("L = %lu R = %lu l = %lu r = %lu\n", L, R, l, r);
			M = (L + R) / 2;
//			printf("M = %lu\n", M);
			if(r > l) {
				lcp_tree_node* parent = lcp_node;
				lcp_node = lcp_node->right_node;

				if(lcp_node == NULL) { // ??
					unsigned long length;
					if (starts_with(pattern + r, string + pos[R] - 1 + r, length ) == 0) {
//						printf(".found at %lu , M = %lu\n", pos[R], R);
						expand(pattern, R, r + length);
						break;
					} else {
//						printf("not found at %lu , M = %lu\n", pos[R], R);
						return -1;
					}
				}

//				printf("distance %lu - %lu lcp value %lu\n", M, R, lcp_node->value);

				if(r > lcp_node->value) { // pos[M] < P // example
					L = M;
					l = lcp_node->value;
					// way in lcp tree is true
				} else if ( r < lcp_node->value) {
					// !!!???
					R = M;
					lcp_node = parent->left_node; // correct way in lcp tree
				} else { // r == lcp_node->value // example
					unsigned long length;
					int result = starts_with(pattern + r, string + pos[M] - 1 + r, length);
					if(result > 0) {
						L = M;
						l = r + length;
						// way in lcp tree is true
					} else if ( result < 0) {
						R = M;
						r += length;
						lcp_node = parent->left_node; // correct way in lcp tree
					} else { // result == 0
//						printf("found at %lu , M = %lu\n", pos[M], M);
						expand(pattern, M, r + length);
						break;
					}
				}
			} else if (r < l) {
				lcp_tree_node* parent = lcp_node;
				lcp_node = lcp_node->left_node;

				if(lcp_node == NULL) {
					unsigned long length;
					if (starts_with(pattern + l, string + pos[L] - 1 + l, length ) == 0) {
//						printf(".found at %lu , M = %lu\n", pos[L], L);
						expand(pattern, L, l + length);
						break;
					} else {
//						printf("not found at %lu , M = %lu\n", pos[L], L);
						return -1;
					}
				}

//				printf("distance %lu - %lu lcp value %lu\n", L, M, lcp_node->value);

				if(l > lcp_node->value) {
					/// !!!
					R = M;
					r = lcp_node->value;
					// way in lcp tree is true
				} else if ( l < lcp_node->value) {
					/// !!!
					L = M;
					lcp_node = parent->right_node; // correct way in lcp tree
				} else { // l == lcp_node->value
					unsigned long length;
					int result = starts_with(pattern + l, string + pos[M] - 1 + l, length);
					if(result > 0) {
						L = M;
						l += length;
						lcp_node = parent->right_node; // correct way in lcp tree
					} else if ( result < 0) {
						R = M;
						r = l + length;
						// way in lcp tree is true
					} else { // result == 0
//						printf("//found at %lu , M = %lu\n", pos[M], M );
						expand(pattern, M, l + length);
						break;
					}
				}

			} else { // l == r

				unsigned long length;
				int result = starts_with(pattern + l, string + pos[M] - 1 + l, length);
				if ( result < 0 ) {
					if(lcp_node == NULL) {
//						printf("...not found at %lu, M = %lu (less)\n", pos[M], M);
						return -1;
					}
					R = M;
//					printf("l==r go to left lcp\n");
					lcp_node = lcp_node->left_node; // go left child
					r += length;
				} else if ( result > 0 ) {
					if(lcp_node == NULL) {
//						printf("...not found at %lu, M = %lu (more)\n", pos[M], M);
						return -1;
					}
					L = M; // lcp == l
//					printf("l==r go to right lcp\n");
					lcp_node = lcp_node->right_node; // go to right child
					l += length;
				} else {
//					printf("!found at %lu , M = %lu\n", pos[M], M );
					expand(pattern, M, l + length);
					break; // found
				}
			}
		}
		return 0;
	}
};

struct char_array {

	char* arr;
	unsigned long size;
	unsigned long capacity;

	char_array(unsigned long initial_capacity = 10) {
		size = 0;
		this->capacity = initial_capacity;
		arr = (char*) malloc(initial_capacity);
		//if(arr == 0)
	}

	~char_array() {
		free(arr);
	}

	void add(const char element) {
		if(size + 1 == capacity) {
			capacity = capacity * 1.5 + 1;
			arr = (char*) realloc(arr, capacity);
//			if(arr == NULL)
		}
		arr[size++] = element;
	}
};

int main() {
/*
	const char* str = "a$";
	suffix_tree st(str);
	st.print();

	FILE* file = fopen("sarr.txt", "w");
	suffix_array sa(st);
	sa.print(file);
	fclose(file);
*/
// i !
//	sa.search_new("abcd");

//	unsigned long len;
//	int s = starts_with("abax", "axbbxbxabxabxabxabbabbxabxbbxabxab$", len);
//	printf("%d %lu\n", s, len);


	char_array text(100);

	int c;
	while ((c = getchar() ) != '\n') {
		text.add(c);
	}
	text.add('$');
	text.add('\0');

	suffix_tree st(text.arr);

	suffix_array sa(st);

	unsigned long n = 1;
	while ( (c = getchar() ) != EOF) {
		char_array pattern;
		if(c == '\n') // empty string
			continue;
		else
			pattern.add(c);
		while( (c = getchar() ) != '\n') {
			pattern.add(c);
		}
		pattern.add('\0');
//		printf("searching %s...\n", pattern.arr);
		printf("%lu: ", n);
		sa.search_new(pattern.arr);
		printf("\n");
		n++;
	}

	return 0;
}

