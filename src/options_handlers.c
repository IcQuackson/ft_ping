
#include "options_handlers.h"

static int parse_int(const char *input, int max)
{
	errno = 0;
	char *endptr = NULL;
	unsigned long long value = strtoull(input, &endptr, 10);

	if (errno == ERANGE || value > (unsigned long long)max) {
		fprintf(stderr, "ping: invalid argument: '%s': must be between 0 and %d\n", input, max);
		exit(EXIT_FAILURE);
	}

	if (endptr == input || *endptr != '\0') {
		fprintf(stderr, "ping: invalid argument: '%s': not a valid number\n", input);
		exit(EXIT_FAILURE);
	}

	return (int)value;
}

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
	arguments->options.ttl = parse_int(optarg, TTL_MAX);
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

void handle_s(t_arguments *arguments, char *optarg)
{
	log_message(INFO, "Option -s selected with value %s", optarg);
	arguments->options.s = parse_int(optarg, MAX_PACKET_SIZE);
}

void handle_default()
{
	display_usage();
	exit(EXIT_FAILURE);
}

void set_default_arguments(t_arguments *arguments)
{
	arguments->options.verbose = 0;
	arguments->options.ttl = DEFAULT_TTL;
	arguments->options.ip_timestamp = 0;
	arguments->options.f = 0;
	arguments->options.l = 0;
	arguments->options.n = 0;
	arguments->options.w = 0;
	arguments->options.W = 0;
	arguments->options.p = 0;
	arguments->options.r = 0;
	arguments->options.s = 56;
	arguments->options.T = 0;
	arguments->options.c = 0;
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