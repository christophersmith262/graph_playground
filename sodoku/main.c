#include <stdio.h>

#include "sodoku.h"

int main(int argc, char *argv[]) {
	graph_t graph;
	FILE *fp;

    sodoku_init(&graph);

	fp = fopen(argv[1], "r");
	sodoku_read(fp, &graph);

	printf("\nPUZZLE:\n");
	printf("------------------------------\n");
    sodoku_print(&graph);
	printf("\n");

	sodoku_color(&graph);

	printf("\nSOLUTION:\n");
	printf("------------------------------\n");
    sodoku_print(&graph);
	printf("\n");

	return 0;
}
