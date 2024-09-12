
#include "options_handlers.h"

void handle_verbose(t_arguments *arguments)
{
	log_message(INFO, "Option -v selected");
	arguments->options.verbose = 1;
}

void handle_help()
{
	display_usage();
	exit(0);
}

void handle_f(t_arguments *arguments)
{
	log_message(INFO, "Option -f selected");
	arguments->options.f = 1;
}

void handle_l(t_arguments *arguments)
{
	log_message(INFO, "Option -l selected");
	arguments->options.l = 1;
}

void handle_ttl(t_arguments *arguments, char *optarg)
{
	log_message(INFO, "Option --ttl selected with value %s", optarg);
	arguments->options.ttl = 1;
	arguments->ttl = atoi(optarg);
}

void handle_ip_timestamp(t_arguments *arguments)
{
	log_message(INFO, "Option --ip-timestamp selected");
	arguments->options.ip_timestamp = 1;
}

void handle_c(t_arguments *arguments, char *optarg)
{
	log_message(INFO, "Option -c selected with value %s", optarg);
	arguments->options.c = atoi(optarg);
}

void handle_default()
{
	display_usage();
	exit(EXIT_FAILURE);
}

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