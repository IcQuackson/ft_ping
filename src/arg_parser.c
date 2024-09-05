#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "arg_parser.h"
#include "logger.h"

void display_usage()
{
    printf("Usage: program [options]\n");
    printf("  -v, --verbose             Increase verbosity\n");
    printf("  -?, --help                Display this help message\n");
    printf("  -f                        Option with f\n");
    printf("  -l                        Option with l\n");
    printf("  -n                        Option with n\n");
    printf("  -w                        Option with w\n");
    printf("  -W                        Option with W\n");
    printf("  -p                        Option with p\n");
    printf("  -r                        Option with r\n");
    printf("  -s                        Option with s\n");
    printf("  -T                        Option with T\n");
    printf("      --ttl                 TTL option\n");
    printf("      --ip-timestamp        IP timestamp option\n");
}


void parse_arguments(int argc, char *argv[], t_arguments *arguments)
{

    int opt;
    int option_index = 0;

    struct option long_options[] =
	{
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, '?'},
        {"ttl", required_argument, 0, 0},
        {"ip-timestamp", no_argument, 0, 0},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "v?flnwWprTsTc:", long_options, &option_index)) != -1)
	{
        switch (opt) {
			case 'v':
				log_message(INFO, "Option -v selected");
				arguments->options.verbose = 1;
				break;
			case '?':
				display_usage();
				exit(0);
			case 'f':
				log_message(INFO, "Option -f selected");
				arguments->options.f = 1;
				break;
			case 'l':
				log_message(INFO, "Option -l selected");
				arguments->options.l = 1;
				break;
			case 'n':
				log_message(INFO, "Option -n selected");
				arguments->options.n = 1;
				break;
			case 'w':
				log_message(INFO, "Option -w selected");
				arguments->options.w = 1;
				break;
			case 'W':
				log_message(INFO, "Option -W selected");
				arguments->options.W = 1;
				break;
			case 'p':
				log_message(INFO, "Option -p selected");
				arguments->options.p = 1;
				break;
			case 'r':
				log_message(INFO, "Option -r selected");
				arguments->options.r = 1;
				break;
			case 's':
				log_message(INFO, "Option -s selected");
				arguments->options.s = 1;
				break;
			case 'T':
				log_message(INFO, "Option -T selected");
				arguments->options.T = 1;
				break;
			case 'c':
				log_message(INFO, "Option -c with value '%s' selected", optarg);
				arguments->options.c = 1;
				arguments->count = atoi(optarg);
				if (arguments->count <= 0) {
                    fprintf(stderr, "Invalid argument for -c. Must be a positive integer.\n");
                    exit(EXIT_FAILURE);
                }
				break;
			case 0: // Case for long options without short equivalents
				if (strcmp(long_options[option_index].name, "ttl") == 0)
				{
					log_message(INFO, "Option --ttl with value '%s' selected", optarg);
					arguments->options.ttl = 1;
					arguments->ttl = atoi(optarg);
					if (arguments->ttl <= 0) {
                        fprintf(stderr, "Invalid argument for --ttl. Must be a positive integer.\n");
                        exit(EXIT_FAILURE);
                    }
				}
				else if (strcmp(long_options[option_index].name, "ip-timestamp") == 0)
				{
					log_message(INFO, "Option --ip-timestamp selected");
					arguments->options.ip_timestamp = 1;
				}
				break;
			default:
				display_usage();
				exit(EXIT_FAILURE);
		}
    }

	parse_host(argc, arguments, argv);
}

void parse_host(int argc, t_arguments *arguments, char *argv[])
{
	if (optind < argc && optind + 1 == argc)
	{
		arguments->host = argv[optind];
		printf("Destination address: %s\n", arguments->host);
	}
	else if (optind < argc)
	{
		printf("ping: usage error: Too many arguments\n");
		exit(EXIT_FAILURE);
	}
	else
	{
		printf("ping: usage error: Destination address required\n");
		exit(EXIT_FAILURE);
	}
}

// Function to print the parsed arguments
void print_arguments(t_arguments *arguments)
{
	if (arguments == NULL) {
		printf("Error: Invalid arguments\n");
		return;
	}
	
	printf("Arguments:\n");
	printf("  verbose: %d\n", arguments->options.verbose);
	printf("  ttl: %d\n", arguments->options.ttl);
	printf("  ip_timestamp: %d\n", arguments->options.ip_timestamp);
	printf("  f: %d\n", arguments->options.f);
	printf("  l: %d\n", arguments->options.l);
	printf("  n: %d\n", arguments->options.n);
	printf("  w: %d\n", arguments->options.w);
	printf("  W: %d\n", arguments->options.W);
	printf("  p: %d\n", arguments->options.p);
	printf("  r: %d\n", arguments->options.r);
	printf("  s: %d\n", arguments->options.s);
	printf("  T: %d\n", arguments->options.T);
	printf("  c: %d\n", arguments->options.c);
	printf("  count: %d\n", arguments->count);
	printf("  ttl: %d\n", arguments->ttl);
}
