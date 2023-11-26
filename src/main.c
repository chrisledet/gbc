#include "gbc.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: gbc <rom filepath>\n");
        return EXIT_FAILURE;
    }
	return gbc_run(argv[1]);
}
