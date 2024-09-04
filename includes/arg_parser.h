#ifndef ARG_PARSER_H
#define ARG_PARSER_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>

typedef struct s_options {
	int verbose;
	int ttl;
	int ip_timestamp;
	int f;
	int l;
	int n;
	int w;
	int W;
	int p;
	int r;
	int s;
	int T;
	int c;
} t_options;

typedef struct s_arguments {
	t_options options;
	char *filename;
	int count;
	int ttl;
} t_arguments;

void display_usage();

void parse_arguments(int argc, char *argv[], t_arguments *arguments);

void print_arguments(t_arguments *arguments);


#endif