// actulaprocess.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include<stdlib.h>
#include "header.h"
#define   BS 16*1024
#define   NUM_BLOCKS 6400
struct node
{
	unsigned int magic_number;
	unsigned int block_size;
	unsigned int numofblocks;
	unsigned int numofemptyblock;
	char bytevector[NUM_BLOCKS];
};
struct metadata
{
	char filename[20];
	unsigned int size;
	unsigned int no_of_blocks;
	unsigned int start_block;
};
void format(char *diskname)
{
	struct node info;
	int i;
	unsigned int nof = 0;
	info.magic_number = 0x444E524D;
	info.block_size = BS;
	info.numofemptyblock = 6399;
	info.numofblocks = 6399;
	for (i = 0; i < 4; i++)
		info.bytevector[i] = '0';
	for (i = 4; i < NUM_BLOCKS; i++)
		info.bytevector[i] = '1';
	char *buffer = (char *)malloc(sizeof(char)*BS);
	memcpy(buffer, &info, sizeof(struct node));
	memcpy(buffer + 6400 + 16, &nof, 4);
	writeblock(buffer, 0);
}
void mount(char *harddisc)
{
	int m = 0x444E524D,num;
	char *buffer = (char *)malloc(sizeof(BS));
	readblock(buffer, 0);
	memcpy(&num, buffer, 4);
	if (num != m)
	{
		printf("file not formatted");
		format(harddisc);
	}
}
int findfreeblock(char arr[])
{
	int i = 0;
	while (arr[i]!= '1')
	{
		i++;
	}
	return i;
}
void strcopy(char *s1, char *s2)
{
	int i = 0;
	while (s2[i] != '\0')
	{
		s1[i] = s2[i];
		i++;
	}
	s1[i] = '\0';
}
unsigned int findsize(char *file)
{
	FILE* fp = fopen(file, "r");
	int res = 0;
	if (fp == NULL) {
		printf("File Not Found!\n");
		return -1;
	}
	fseek(fp, 0, SEEK_END);
		res = ftell(fp);
	return res;
}
void split(char *arg, char *src, char *dest)
{
	int index, i,j=0;
	index = strcspn(arg, ",");
	i = index + 1;
	while (arg[i] != '\0')
	{
		dest[j] = arg[i];
		j++; i++;
	}

	dest[j] = '\0';
	i = 0;
	while (i < index)
	{
		src[i] = arg[i];
		i++;
	}
	src[i] = '\0';
}
void copytofs(char *arg,char *harddisk)
{
	unsigned int start, filesize, blocksreq, count = 0,n,nextpos,address,num,emptyblocks,k,magic,flag=0;
	char arr[6400];
	char src[100], dest[100];
	struct metadata newfile;
	int i = 0;
	char *buffer = (char *)malloc(BS);
	char *buffer1 = (char *)malloc(BS);
	split(arg, src, dest);
	readblock(buffer, 0);
	memcpy(&magic, buffer, 4);
	memcpy(&n, buffer + 16+6400, 4);
	k = n + 1;
	memcpy(arr, buffer + 16, NUM_BLOCKS);
	memcpy(&emptyblocks, buffer + 12, 4);
	filesize = findsize(src);
	/*if (filesize % BS == 0)
		blocksreq = filesize / BS;
	else
		blocksreq = (filesize / BS) + 1;*/
		strcopy(newfile.filename, dest);
		newfile.size = filesize;
		newfile.no_of_blocks = 0;
		start = findfreeblock(arr);
		emptyblocks--;
		arr[start] = '0';
		newfile.start_block = start;
	   FILE *sfp;
	   address = start;
	   sfp = fopen(src, "rb+");
	   if (sfp != NULL)
	   {
		   while (fread(buffer, 1, BS, sfp) > 0)
		   {
			   if (flag==0&&filesize<BS)
			   {
				   writeblock(buffer, start);
				   newfile.no_of_blocks += 1;
				   break;
			   }
			   else
			   {
				   flag = 1;
				   start = findfreeblock(arr);
				   arr[start] = '0';
				   newfile.no_of_blocks += 1;
				   emptyblocks--;
				   writeblock(buffer, start);
				   memcpy(buffer1 + count * 4, &start, 4);
				   count++;
				   filesize -= BS;
			   } 
			   //if (n == EOF)return;
		   }
		   if (flag==1)
			   writeblock(buffer1, address);
		   readblock(buffer,0);
		   memcpy(buffer + 12, &emptyblocks, 4);
		   memcpy(buffer + 16, arr, NUM_BLOCKS);
		   memcpy(buffer + sizeof(struct node),&k, 4);
		   memcpy(buffer + sizeof(struct node) + 4 + (n)*sizeof(struct metadata), &newfile, sizeof(struct metadata));
		   writeblock(buffer, 0);	  
}
}
void copyfromfs(char *arg, char *harddisk)
{
	int num,i,start,size,block,count=0;
	char src[100], dest[100];
	split(arg, src, dest);
	FILE *fptr;
	fptr = fopen(dest, "wb");
	struct metadata node;
	char *buffer = (char *)malloc(BS);
	char *buffer1 = (char *)malloc(BS);
	readblock(buffer, 0);
	memcpy(&num, buffer + sizeof(struct node), 4);
	for (i = 0; i < num; i++)
	{
		memcpy(&node, buffer + sizeof(struct node) + 4 + i*sizeof(struct metadata), sizeof(struct metadata));
		if (!strcmp(node.filename, src))
			break;
	}
	start = node.start_block;
	size = node.size;
	if (node.no_of_blocks == 1)
	{
		readblock(buffer, start);
		fwrite(buffer, 1, size, fptr);
	}
	else
	{
		while (size != 0)
		{
			readblock(buffer, start);
			memcpy(&block, buffer + count * 4, 4);
			readblock(buffer1, block);
			if (size <= BS)
			{
				fwrite(buffer1, 1, size, fptr);
				size = 0;
			}
			else
			{
				fwrite(buffer1, 1, BS, fptr);
				size -= BS;
			}
			count++;
		}
	}
	fclose(fptr);
}
void list(char *harddisk)
{
	int nooffiles,i;
	struct metadata var;
	char * buffer = (char *)malloc(BS);
	readblock(buffer, 0);
	memcpy(&nooffiles, buffer + sizeof(struct node), 4);
	if (nooffiles ==0)
	{
		printf("disk is empty");
		return;
	}
	for (i = 0; i < nooffiles; i++)
	{
		memcpy(&var, buffer + sizeof(struct node) + 4 + i*sizeof(struct metadata), sizeof(struct metadata));
		printf("%s\n", var.filename);
	}
}
void debug(char *harddisk)
{
	unsigned int magicno, blocksize, numofblocks, numofemp,nooffiles,i;
	struct metadata var;
	char vector[6400];
	char * buffer = (char *)malloc(BS);
	readblock(buffer, 0);
	struct node buf;
	memcpy(&magicno, buffer, 4);
	memcpy(&blocksize, buffer+4, 4);
	memcpy(&numofblocks, buffer+8, 4);
	memcpy(&numofemp, buffer+12, 4);
	memcpy(vector, buffer+16, 6400);
	printf("magicno=%d\n",magicno);
	printf("blocksize=%d\n", blocksize);
	printf("numofblocks=%d\n", numofblocks);
	printf("numofemp=%d\n", numofemp);
	printf("freeblocks:");
	for (i = 0; i < numofblocks; i++)
	{
		printf("%c", vector[i]);
	}
	memcpy(&nooffiles, buffer + sizeof(struct node), 4);
	printf("nooffiles=%d\n", nooffiles);
	for (i = 0; i < nooffiles; i++)
	{
		memcpy(&var, buffer + sizeof(struct node) + 4 + i*sizeof(struct metadata), sizeof(struct metadata));
		printf("%s %d %d %d\n", var.filename, var.size, var.no_of_blocks, var.start_block);
	}

}
void deletefile(char *src, char *disk)
{
	int num, i, start, size, block, count = 0,pos,empty;
	struct metadata node;
	char *buffer = (char *)malloc(BS);
	char *buffer1 = (char *)malloc(BS);
	readblock(buffer, 0);
	memcpy(&num, buffer + sizeof(struct node), 4);
	for (i = 0; i < num; i++)
	{
		memcpy(&node, buffer + sizeof(struct node) + 4 + i*sizeof(struct metadata), sizeof(struct metadata));
		if (!strcmp(node.filename, src))
			pos = i;
	}
	memcpy(&node, buffer + sizeof(struct node) + 4 + (num-1)*sizeof(struct metadata), sizeof(struct metadata));
	memcpy(buffer + sizeof(struct node) + 4 + pos*sizeof(struct metadata),&node, sizeof(struct metadata));
	num--;
	memcpy(buffer + sizeof(struct node),&num, 4);
	writeblock(buffer, 0);
}
void get_command(char cmd[255])
{
	int index, i, argpos, j = 0;
	cmd[strcspn(cmd, "\n")] = '\0';
	char arg[100];
	char diskname[100];
	index = strcspn(cmd, " ");
	i = index;
	while (cmd[i] != '\0'&&cmd[i] == ' ')
	{
		i++;
	}
	if (cmd[i] != '\0')
	{
		argpos = i;
		while (cmd[argpos] != '\0')
		{
			arg[j++] = cmd[argpos];
			argpos++;
		}
		arg[j] = '\0';
	}
	cmd[index] = '\0';
	if (!strcmp(cmd, "mount"))
	{
		strcopy(diskname, arg);
		mount(arg);
	}
	if (!strcmp(cmd, "copyto"))
		copytofs( arg, diskname);
    if (!strcmp(cmd, "copyfrom"))
		copyfromfs( arg, diskname);
	if (!strcmp(cmd, "format"))
		format( diskname);
	if (!strcmp(cmd, "ls"))
		list(diskname);
	if (!strcmp(cmd, "debug"))
		debug( diskname);
	if (!strcmp(cmd, "delete"))
		deletefile(arg, diskname);
}
int main()
{
	struct node info;
	int i;
	unsigned int nof = 0;
	char cmd[255];
	/*info.magic_number = 0x444E524D;
	info.block_size = BS;
	info.numofemptyblock = 6399;
	info.numofblocks = 6399;
	for (i = 0; i < 4;i++)
		info.bytevector[i] = '0';
	for (i = 4; i < NUM_BLOCKS; i++)
		info.bytevector[i] = '1';*/
    //format("harddisk.hdd");
	//copytofs("adminloged.png", "imgcpy.png", info, "harddisk.hdd");
    //copytofs("temp.txt","newtem.txt",info,"harddisk.hdd");
	//copytofs("report.pdf", "reportcpy.pdf", info, "harddisk.hdd");
	//list("harddisk.hdd");
	//copyfromfs("image.png", "newim.png", "harddisk.hdd");
	//deletefile("reportcpy.pdf", "harddisk.hdd");
	//debug("harddisk.hdd");
	do
	{
		printf("\n>");
		fgets(cmd, 255, stdin);
		get_command(cmd);
	} while (strcmp(cmd, "exit"));
	return 0;
}

