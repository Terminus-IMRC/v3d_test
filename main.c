#include "v3d.h"
#include "mailbox.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char *progname = NULL;
static _Bool is_wanted_exit = 0;

void usage()
{
	printf("Usage: read: %s -r -n REGISTER_NAME\n", progname);
	printf("       write: %s -w VALUE -n REGISTER_NAME\n", progname);
	printf("Read value from V3D register REGISTER_NAME.\n");
	printf("Or write value VALUE to V3D register REGISTER_NAME.\n");
}

void exit_handler()
{
	if (!is_wanted_exit)
		usage();
}

int main(int argc, char *argv[])
{
	int opt;
	char *name = NULL;
	uint32_t *v3d_p;
	enum {
		E_NA, E_R, E_W
	} rw = E_NA;
	v3d_field_name_t reg;
	uint32_t val = 0;

	progname = argv[0];

	atexit(exit_handler);

	while ((opt = getopt(argc, argv, "rw:n:")) != -1) {
		switch (opt) {
			case 'r':
				if (rw == E_NA)
					rw = E_R;
				else {
					fprintf(stderr, "rw flags are specified more than once\n");
					exit(EXIT_FAILURE);
				}
				break;
			case 'w':
				if (rw == E_NA) {
					rw = E_W;
					val = strtoul(optarg, NULL, 0);
				} else {
					fprintf(stderr, "rw flags are specified more than once\n");
					exit(EXIT_FAILURE);
				}
				break;
			case 'n':
				name = strdup(optarg);
				break;
			default:
				fprintf(stderr, "invalid option: -%c\n", opt);
				exit(EXIT_FAILURE);
		}
	}

	if (optind < argc) {
		fprintf(stderr, "unexpected arguments after options\n");
		exit(EXIT_FAILURE);
	}

	if (name == NULL) {
		fprintf(stderr, "name is not specified\n");
		exit(EXIT_FAILURE);
	}

	if(rw == E_NA) {
		fprintf(stderr, "rw flag is not specified\n");
		exit(EXIT_FAILURE);
	}

	v3d_init();
	reg = v3d_str_to_reg(name);
	free(name);
	v3d_p = mapmem_cpu(BUS_TO_PHYS(v3d_peripheral_addr()), V3D_LENGTH);

	switch (rw) {
		case E_R:
			printf("0x%x\n", v3d_read(v3d_p, reg));
			break;
		case E_W:
			v3d_write(v3d_p, reg, val);
			break;
		default:
			fprintf(stderr, "%s:%d: internal error\n", __FILE__, __LINE__);
			exit(EXIT_FAILURE);
	}

	unmapmem_cpu(v3d_p, V3D_LENGTH);
	v3d_finalize();

	is_wanted_exit = !0;
	return 0;
}
