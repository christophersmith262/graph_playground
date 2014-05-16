/**
 * sodoku.h
 *
 * Author: Christopher Smith
 *
 * Description: Solves a sodoku puzzle as a graph coloring problem.
 */

#ifndef __SODOKU_H
#define __SODOKU_H

/**
 * Represents a sodoku puzzles as a graph of 81 vertexes
 * to be colored.
 */
typedef struct {
	char nodes[9*9];
	int colored;
} graph_t;

/**
 * Initialize a sodoku graph
 */
void sodoku_init(graph_t *graph);

/**
 * Read sodoku graph from fp (as string).
 */
int sodoku_read(FILE *fp, graph_t *graph);

/**
 * Formatted output of sodoku graph to stdout (as string).
 */
void sodoku_print(graph_t *graph);

#endif
