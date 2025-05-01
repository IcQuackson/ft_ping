#include <stdio.h>
#include <stdlib.h>

#include "arg_parser.h"
#include "ft_ping.h"

int main(int argc, char *argv[])
{
	t_arguments arguments;

    signal(SIGINT, sigint_handler);
    memset(&arguments, 0, sizeof(t_arguments));
    parse_arguments(argc, argv, &arguments);
    check_arguments(&arguments);
	//print_arguments(&arguments);
    ft_ping(&arguments);
    return EXIT_SUCCESS;
}
