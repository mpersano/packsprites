#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>

#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>

#include <vector>

#include "sprite.h"
#include "pack.h"
#include "png_util.h"
#include "panic.h"

namespace {
}


static void
usage(const char *argv0)
{
	fprintf(stderr,
		"usage: packsprites [options] sheetname spritepath\n"
		"\n"
		"options:\n"
		"-b	size of border around the packed sprites, in pixels (default: 2)\n"
		"-w	spritesheet width (default: 256)\n"
		"-h	spritesheet height (default: 256)\n");

	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	int c;
	int border = 2;
	int sheet_width = 256;
	int sheet_height = 256;
	std::string texture_path_base = ".";

	while ((c = getopt(argc, argv, "b:w:h:t:")) != EOF) {
		switch (c) {
			case 'b':
				border = atoi(optarg);
				break;

			case 'w':
				sheet_width = atoi(optarg);
				break;

			case 'h':
				sheet_height = atoi(optarg);
				break;

			case 't':
				texture_path_base = optarg;
				break;
		}
	}

	const char *sheet_name = argv[optind];

	std::vector<std::unique_ptr<sprite_base>> sprites;

	for (int i = optind + 1; i < argc; i++) {
		const char *dir_name = argv[i];

		DIR *dir = opendir(dir_name);
		if (!dir)
			panic("failed to open %s: %s\n", dir_name, strerror(errno));

		while (dirent *de = readdir(dir)) {
			const char *name = de->d_name;
			size_t len = strlen(name);

			if (len >= 4 && !strcmp(name + len - 4, ".png")) {
				char path[PATH_MAX];
				sprintf(path, "%s/%s", dir_name, name);
				sprites.push_back(std::unique_ptr<sprite_base> { new sprite(name, png_read(path)) });
			}
		}

		closedir(dir);
	}

	pack(sprites,
		sheet_name,
		sheet_width, sheet_height,
		border,
		texture_path_base.c_str());
}
