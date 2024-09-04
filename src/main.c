#include <stdio.h>
#include <stdlib.h>

#include "arg_parser.h"

int main(int argc, char *argv[]) {
	t_arguments arguments = {0};

    parse_arguments(argc, argv, &arguments);

	//print_arguments(&arguments);

    printf("Program logic execution begins here...\n");

    return 0;
}
