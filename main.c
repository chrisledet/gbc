#include <stdio.h>
#include <stdlib.h>
#include <gbc.h>


int main(int argc, const char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: gbc <rom filepath>\n");
        return EXIT_FAILURE;
    }

	gbc_run(argv[1]);

	return EXIT_SUCCESS;
}
