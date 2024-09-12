#ifndef OPTIONS_HANDLER_H
#define OPTIONS_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "arg_parser.h"
#include "logger.h"

// forward declaration of t_arguments
typedef struct s_arguments t_arguments;

void handle_verbose(t_arguments *arguments);
void handle_help();
void handle_f(t_arguments *arguments);
void handle_l(t_arguments *arguments);
void handle_ttl(t_arguments *arguments, char *optarg);
void handle_ip_timestamp(t_arguments *arguments);
void handle_c(t_arguments *arguments, char *optarg);
void handle_default();
void display_usage();

#endif