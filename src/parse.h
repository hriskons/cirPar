#ifndef PARSE_H
#define PARSE_H

#include <stdio.h>
#include "node_list.h"

#define MAX_LINE_SIZE 1000

int check_netlist_file(FILE *input_file,LIST* list);

/*
 * Parse netlist in file : filename
 *
 * returns : 1 OK , 0 FAIL
 */
int parse_netlist(char* filename , LIST* list);

#endif