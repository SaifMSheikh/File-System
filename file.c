#include <string.h>
#include "inode.h"
#include "file.h"
#include "disk.h"
// Find Directory, Given Parent & Relative Path
uint16_t dir_lookup(inode_s* dir,char* target_path) {
	// Validate Inputs
	if (!dir->valid||dir->info->type!=I_DIRE) {
		printf("Invalid Root Inode");
		return IMAX(dir->dev->info);
	}
	// Parse Target Path For Subdirectory
	char sub_path[255]="\0";
	char next_name[DIRENT_NAME_LEN]="\0";
	for (int i=0;i<strlen(target_path);++i) {
		if (target_path[i]=='/') {
			strncpy(sub_path,&target_path[i+1],strlen(target_path)-i+1);
			sub_path[strlen(target_path)-i+1]='\0';
			strncpy(next_name,target_path,i);
			next_name[i]='\0';
			break;
		}
	}
	// If Found, Search Next Directory
	if (sub_path[0]!='\0') {
		// Find Next Directory's Entry
		inode_s next_dir;
		next_dir.valid=false;
		// 1 - Search Direct Data
		dirent_s* entry=(dirent_s*)dir->info->addr;
		for (int i=0;(i<dir->info->size)&&(i<NDIRENT_DIRECT);++i) 
			if (!strcmp(entry[i].name,next_name)) 
				next_dir=_inode_get(dir->dev,entry[i].inum);
		// 2 - Search Indirect Data
		entry=(dirent_s*)&(dir->dev->mem_start[BLOCK_SIZE*(dir->dev->info.data_start+dir->info->addr[NDIRENT_DIRECT])]);
		for (int i=0;i<(dir->info->size-NDIRENT_DIRECT);++i)
			if (!strcmp(entry[i].name,next_name)) 
				next_dir=_inode_get(dir->dev,entry[i].inum);
		// Search Next Directory If Found
		if (next_dir.valid)
			return dir_lookup(&next_dir,sub_path);
		// Not Found
		printf("Directory Not Found : %s\n",next_name);
		return IMAX(dir->dev->info);
	}
	// Otherwise, Search For Match
	dirent_s* entry=(dirent_s*)dir->info->addr;
	// 1 - Search Direct Data
	for (int i=0;(i<dir->info->size)&&(i<NDIRENT_DIRECT);++i)
		if (!strcmp(entry[i].name,target_path))
			return entry[i].inum;
	// 2 - Search Indirect Data
	int size=(int)dir->info->size-NDIRENT_DIRECT;
	entry=(dirent_s*)&(dir->dev->mem_start[BLOCK_SIZE*(dir->dev->info.data_start+dir->info->addr[NDIRENT_DIRECT])]);
	for(int i=0;(i<size)&&(i<NDIRENT_INDIRECT);++i)
		if (!strcmp(entry[i].name,target_path))
			return entry[i].inum;
	// Not Found
	return IMAX(dir->dev->info);
}
// Create Directory At Relative Path From Parent
uint16_t dir_create(inode_s* dir,char* path) {
	// Validate Input
	if (!dir->valid) {
		printf("Invalid Inode\n");
		return IMAX(dir->dev->info);
	}
	// Check If Taken
	if (dir_lookup(dir,path)!=IMAX(dir->dev->info))
		return IMAX(dir->dev->info);
	// Get Parent Directory & Relative Path
	char* parent_path="";
	char* target_name="";
	for(int i=strlen(path)-1;i>=0;--i) {
		if (path[i]=='/') {
			strncpy(parent_path,path,i);
			strncpy(target_name,&path[i+1],strlen(path)-i+1);
			break;
		}
	}
	// Validate Target Name
	if (strlen(target_name)>=DIRENT_NAME_LEN) {
		printf("Name Too Long\n");
		return IMAX(dir->dev->info);
	}
	// If Not Found, Use Current Directory As Parent
	dnode_s* parent_dir;
	if (!strcmp(parent_path,""))
		parent_dir=dir->info;
	// Otherwise, Fetch Parent Directory
	else parent_dir=_disk_inode(dir->dev,dir_lookup(dir,parent_path));
	if (parent_dir==NULL)
		return IMAX(dir->dev->info);
	// Update Parent Directory Data
	if (parent_dir->size>=NDIRENT) {
		printf("Parent Directory (%s) Full\n",parent_path);
		return IMAX(dir->dev->info);
	}
	dirent_s* entry;
	if (parent_dir->size<NDIRENT_DIRECT) {
		// Update Direct Data
		entry=(dirent_s*)&(parent_dir->addr[parent_dir->size]);
	} else {
		// Update Indirect Data
		entry=(dirent_s*)&(dir->dev->mem_start[BLOCK_SIZE*(dir->dev->info.data_start+parent_dir->addr[NDIRENT_DIRECT])]);
		entry+=(parent_dir->size-NDIRENT_DIRECT);
	}
	strcpy(entry->name,target_name);
	parent_dir->size++;
	// Allocate Disk Space
	entry->inum=_disk_inode_alloc(dir->dev);
	return entry->inum;
}
