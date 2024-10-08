#ifndef ARG_PARSER_H
#define ARG_PARSER_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>
#include "options_handlers.h"

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
	char *host;
	char *filename;
	int count;
	int ttl;
} t_arguments;


void parse_arguments(int argc, char *argv[], t_arguments *arguments);

void parse_host(int argc, t_arguments *arguments, char *argv[]);

void print_arguments(t_arguments *arguments);

void parse_host(int argc, t_arguments *arguments, char *argv[]);

void check_arguments(t_arguments *arguments);

#endif