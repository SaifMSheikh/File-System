#ifndef DISK_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define BLOCK_SIZE (4 * 1024)

typedef struct disk_s {
	const char* file_name;
	uint8_t* mem_start;
	size_t mem_size;
	FILE* file;
} disk_s;

bool is_valid(const disk_s*,const char*);
bool open_disk(disk_s*,const char*);
bool close_disk(disk_s*);

#endif
