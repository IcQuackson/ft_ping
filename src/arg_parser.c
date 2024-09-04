#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "arg_parser.h"

// Function to display usage
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

// Function to parse command-line arguments
void parse_arguments(int argc, char *argv[], t_arguments *arguments)
{

    int opt;
    int option_index = 0;

    // Array of long options
    struct option long_options[] =
	{
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, '?'},
        {"ttl", required_argument, 0, 0},
        {"ip-timestamp", no_argument, 0, 0},
        {0, 0, 0, 0}
    };

    // Parse the command-line arguments
    while ((opt = getopt_long(argc, argv, "v?flnwWprTsTc:", long_options, &option_index)) != -1)
	{
        switch (opt) {
			case 'v':
				printf("Verbose mode enabled\n");
				arguments->options.verbose = 1;
				break;
			case '?':
				display_usage();
				exit(0);
			case 'f':
				printf("Option -f selected\n");
				arguments->options.f = 1;
				break;
			case 'l':
				printf("Option -l selected\n");
				arguments->options.l = 1;
				break;
			case 'n':
				printf("Option -n selected\n");
				arguments->options.n = 1;
				break;
			case 'w':
				printf("Option -w selected\n");
				arguments->options.w = 1;
				break;
			case 'W':
				printf("Option -W selected\n");
				arguments->options.W = 1;
				break;
			case 'p':
				printf("Option -p selected\n");
				arguments->options.p = 1;
				break;
			case 'r':
				printf("Option -r selected\n");
				arguments->options.r = 1;
				break;
			case 's':
				printf("Option -s selected\n");
				arguments->options.s = 1;
				break;
			case 'T':
				printf("Option -T selected\n");
				arguments->options.T = 1;
				break;
			case 'c':
				printf("Option -c with value '%s' selected\n", optarg);
				arguments->options.c = 1;
				arguments->count = atoi(optarg);
				break;
			case 0: // Case for long options without short equivalents
				if (strcmp(long_options[option_index].name, "ttl") == 0)
				{
					printf("Option --ttl with value '%s' selected\n", optarg);
					arguments->options.ttl = 1;
					arguments->ttl = atoi(optarg);
				}
				else if (strcmp(long_options[option_index].name, "ip-timestamp") == 0)
				{
					printf("Option --ip-timestamp selected\n");
					arguments->options.ip_timestamp = 1;
				}
				break;
			default:
				display_usage();
				exit(EXIT_FAILURE);
		}
    }

    // Remaining command-line arguments (if any) can be processed here
    if (optind < argc)
	{
        printf("Non-option arguments:\n");
        while (optind < argc)
		{
            printf("%s\n", argv[optind++]);
        }
		// TODO limit to 1 argument?
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
