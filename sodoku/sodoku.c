/**
 * sodoku.c
 *
 * Author: Christopher Smith
 *
 * Description: Solves a sodoku puzzle as a graph coloring problem.
 */

#include <stdio.h>
#include <stdlib.h>

#include "sodoku.h"

typedef struct {
	int x, y;
	int count;
} sodoku_set_member;

/**
 * Call this after creating any graph to initialize it.
 */
void sodoku_init(graph_t *graph) {
	int i, j;
	for(i = 0; i < 9; i++) {
		for(j = 0; j < 9; j++) {
			graph->nodes[i*9 + j] = '-';
		}
	}
	graph->colored = 0;
}

/**
 * Getter function for a vertexes color.
 */
char sodoku_node_value(graph_t *graph, int x, int y) {
	return graph->nodes[x*9 + y];
}

/**
 * Displays the graph in a nice grid format to the stdout.
 */
void sodoku_print(graph_t *graph) {
	int i, j;
	for(i = 0; i < 9; i++) {
		for(j = 0; j< 9; j++ ) {
			printf("%c ", sodoku_node_value(graph, i, j));
		}
		printf("\n");
	}
}


/**
 * Initializes the adjacency set.
 */
void sodoku_init_relationship_map(char *adj) {
	int i;
	for(i = 0; i <= 9; i++) {
		adj[i] = 0;
	}
}

/**
 * Adds an entry to the adjency set.
 */
void sodoku_add_relationship_mapping(graph_t *graph, int from_x, int from_y, int to_x, int to_y, char *adj) {
	char c;
	if(from_x != to_x || from_y != to_y) {
		if ((c = sodoku_node_value(graph, to_x, to_y)) != '-') {
			adj[c - 0x30 - 1] = 1;
		}
	}
}

/**
 * Fills in an adjaceny set.
 */
void sodoku_fetch_adj_values(graph_t *graph, int x, int y, char *adj) {
	int i, j;
	int base_x, base_y;

	sodoku_init_relationship_map(adj);

	for(i = 0; i < 9; i++) {
		sodoku_add_relationship_mapping(graph, x, y, x, i, adj);
		sodoku_add_relationship_mapping(graph, x, y, i, y, adj);
	}

	base_x = (x / 3) * 3;
	base_y = (y / 3) * 3;
	for(i = base_x; i < base_x + 3; i++) {
		for(j = base_y; j < base_y + 3; j++) {
			sodoku_add_relationship_mapping(graph, x, y, i, j, adj);
		}
	}
}

/**
 * Fills in an adjaceny set and returns its cardinality.
 */
int sodoku_count_adj(graph_t *graph, int x, int y) {
	int i;
	char adj[9];
	int count = 0;

	sodoku_fetch_adj_values(graph, x, y, adj);
	for(i = 0; i < 9; i++) {
		if(adj[i]) {
			count++;
		}
	}

	return count;
}

/**
 * Iterator for nodes that orders results by the number of possible colorings
 * available from least to greatest.
 */
int sodoku_fetch_next_node(graph_t *graph, int *p_x, int *p_y) {
	int i, j;
	int rtn = 0;
	int p;
	for(i = 0; i < 9; i++) {
		for(j = 0; j < 9; j++) {
			if(sodoku_node_value(graph, i, j) == '-') {
				p = sodoku_count_adj(graph, i, j);
				if((i*9 + j) > (*p_x*9 + *p_y)) {
					if(p > rtn && p != 9) {
						rtn = 1;
						*p_x = i;
						*p_y = j;
					}
				}
			}
		}
	}
	return rtn;
}

/**
 * Returns 1 if the vertex at x,y has an interferance,
 * otherwise returns 0.
 */
int sodoku_interferes(graph_t *graph, int x, int y) {
	char adj[9];
	char c;
	int rtn = 0;

	sodoku_fetch_adj_values(graph, x, y, adj);
	if((c = sodoku_node_value(graph, x, y)) != '-') {
		rtn = (int)adj[c - 0x30 - 1];
	}

	return rtn;
}

/**
 * Set a vertex color.
 */
int sodoku_set(graph_t *graph, int x, int y, char c) {
	int rtn = 0;
	char old = graph->nodes[x*9 + y];

	// Update vertex
	graph->nodes[x*9 + y] = c;

	// If we are "uncoloring" a vertex, decreased 'colored' count
	if(c == '-' && old != c) {
		graph->colored--;
	}

	// If we are "coloring" a vertex increase the count, but
	// If it is an invalid coloring, abort.
	else if(c != '-' && old == '-') {
		if(sodoku_interferes(graph, x, y)) {
			graph->nodes[x*9 + y] = old;
		} else {
			graph->colored++;
			rtn = 1;
		}
	}
	return rtn;
}

/**
 * Reads data from FILE stream to the graph.
 *
 * Data will be read as a string.  '-' is an
 * empty cell and '0'-'9' are interpretted literally.
 *
 * All other characters ignored.
 */
int sodoku_read(FILE *fp, graph_t *graph) {
	int rtn = 0;
	int read = 0;
	char c;
	int x,y;

	while(fscanf(fp, "%c", &c) > 0) {
		if(isdigit(c) || c == '-') {
			x = read / 9;
			y = read % 9;
			sodoku_set(graph, x, y, c);
			read++;
		}
	}

	return read == 81;
}

/**
 * This heuristic uses a set membership test to simplify the graph.
 *
 * For each block in the sodoku we establish a set for each number 1-9.
 * We then iterate through each vertex in the graph and fill the sets
 * for the block containing that vertex based on whether that vertex
 * can "support" the number associated with the set.
 *
 * At the end of this process, if any vertex is the only member of a block
 * that can support a number, we color this vertex with that number.
 */
int sodoku_quick_color(graph_t *graph) {
	int rtn = 0;
	int i, j, k;
    sodoku_set_member colors[9][9]; // the first array index is the block
                                    // the second array is the number
	char adj[9];
	int base_x, base_y;

	do {

		// Assume no coloring was achieved initially.
		rtn = 0;

		// Initialize the sets
		for(i = 0; i < 9; i++) {
			for(j = 0; j < 9; j++) {
				colors[i][j].count = 0;
			}
		}

		// Iterate through each vertex to build the sets
		for(i = 0; i < 9; i++) {
			for(j = 0; j < 9; j++) {

				// Get the coordinates of the block this vertex belongs to
				base_x = (i / 3);
				base_y = (j / 3);

				// Check if the vertex can be colored with each of the
				// 9 colors.  If it can we mark that color as being associated
				// with the current vertex and increase the count of how
				// many vertexes can be colored with color k.
				if(graph->nodes[i*9+j] == '-') {
					sodoku_fetch_adj_values(graph, i, j, adj);
					for(k = 0; k < 9; k++) {
						if(!adj[k]) {
							colors[base_x*3 + base_y][k].x = i;
							colors[base_x*3 + base_y][k].y = j;
							colors[base_x*3 + base_y][k].count++;
						}
					}
				}
			}
		}

		// Go through the color sets for each block
		for(i = 0; i < 9; i++) {
			for(j = 0; j < 9; j++) {

				// If there is a color set that can only go in exactly one vertex
				// then color that vertex with that color.
				if(colors[i][j].count == 1) {
					sodoku_set(graph, colors[i][j].x, colors[i][j].y, j + 1 + 0x30);
					rtn = graph->colored;
				}
			}
		}

	} while(rtn);


	return rtn;
}

/**
 * Saves the state of a graph so we don't have to worry
 * about tracking changes in the middle of the search.
 */
graph_t *sodoku_push_state(graph_t *graph) {
	int i;
	graph_t *rtn = (graph_t *)malloc(sizeof(graph_t));

	for(i = 0; i < 81; i++) {
		rtn->nodes[i] = graph->nodes[i];
	}

	rtn->colored = graph->colored;

	return rtn;
}

/**
 * Restores the state of a graph.
 */
void sodoku_pop_state(graph_t *to, graph_t *from) {
	int i;

	if(to->colored != 81) {
		for(i = 0; i < 81; i++ ) {
			to->nodes[i] = from->nodes[i];
		}

		to->colored = from->colored;
	}

	free(from);
}

/**
 * Perform 9-coloring of the sodoku graph.
 */
int sodoku_color(graph_t *graph) {
	int i;
	int x, y;
	int rtn = 0;
	char adj[9];
	graph_t *top;

	x = y = -1;

	// Save the current graph state in case this coloring
	// attempt leads to a dead end.
	top = sodoku_push_state(graph);

	// Try to simplify the problem at every step so we don't
	// end up doing a naive exaughstive search.
	sodoku_quick_color(graph);

	// If every vertex is already colored we succeeded
	if(graph->colored == 81) {
		rtn = graph->colored;
	} else {

		// Fetch the next vertex with the lowest number of color
		// possabilities to attempt to color
		while(sodoku_fetch_next_node(graph, &x, &y)) {

			// Attempt to color this vertex each possible color
			sodoku_fetch_adj_values(graph, x, y, adj);
			for(i = 0; i < 9; i++) {

				if(!adj[i]) {

					// Color the vertex and check if we're finished
					if(sodoku_set(graph, x, y, i + 1 + 0x30) ) {
						if(sodoku_color(graph)) {
							rtn = graph->colored;
							break;
						}
					}

					// If it failed, backtrack.
					sodoku_set(graph, x, y, '-');
				}
			}
		}
	}

	// The quick-color heuristic destroys the state we entered
	// with so we have to restore this before letting the search
	// continue.
	sodoku_pop_state(graph, top);

	return rtn;
}
