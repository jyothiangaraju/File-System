#include "stdafx.h"
#include<stdlib.h>
#include<string.h>
int blocksize;
void readblock(void *buffer, int block)
{
	FILE *fptr;
	fptr = fopen("harddisk.hdd", "rb+");
	fseek(fptr, block * 16384, SEEK_SET);
	fread(buffer, 16384, 1, fptr);
	fclose(fptr);
}
void writeblock(void *buffer, int block)
{
	FILE *fptr;
	fptr = fopen("harddisk.hdd", "rb+");
	fseek(fptr, block * 16384, SEEK_SET);
	fwrite(buffer, 16384, 1, fptr);
	fclose(fptr);
}
int tonum(char str[30])
{
	int index = 0, value = 0;
	while (str[index] != '\0')
	{
		value = value * 10 + (str[index] - '0');
		index++;
	}
	return value;
}
unsigned int init(char *filename, int blocksize)
{

	unsigned int no_of_blocks = 100 * 1024 * 1024 / blocksize;
	return no_of_blocks;
}
