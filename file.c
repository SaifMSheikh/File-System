#include <string.h>
#include "inode.h"
#include "file.h"
#include "disk.h"

uint16_t dir_lookup(inode_s* dir,char* target_path) {
	// Validate Inputs
	if (!dir->valid||dir->info->type!=I_DIRE) {
		printf("Invalid Root Inode");
		return IMAX(dir->dev->info);
	}
	// Parse Target Path For Subdirectory
	char* sub_path="";
	char* next_name="";
	for (int i=0;target_path[i]!='\0';++i) {
		if (target_path[i]=='/') {
			memcpy(sub_path,&target_path[i+1],strlen(target_path)-i+1);
			memcpy(next_name,target_path,i);
			break;
		}
	}
	// If Found, Search Next Directory
	if (strcmp(sub_path,"")) {
		// Find Next Directory's Entry
		dirent_s* entry=(dirent_s*)dir->info->addr;
		for (int i=0;(i<dir->info->size)&&(i<NENTRY);++i) {
			if (!strcmp(entry[i].name,next_name)) {
				inode_s next_dir=_inode_get(dir->dev,entry[i].inum);
				return dir_lookup(&next_dir,sub_path);
			}
		}
		// Not Found
		printf("Directory Not Found : %s\n",next_name);
		return IMAX(dir->dev->info);
	}
	// Otherwise, Search For Match
	dirent_s* entry=(dirent_s*)dir->info->addr;
	for (int i=0;(i<dir->info->size)&&(1<NENTRY);++i) {
		if (!strcmp(entry[i].name,target_path))
			return entry[i].inum;
	}
	// Not Found
	return IMAX(dir->dev->info);
}
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
			memcpy(parent_path,path,i);
			memcpy(target_name,&path[i+1],strlen(path)-i+1);
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
	if (parent_dir->size>=NENTRY) {
		printf("Parent Directory (%s) Full\n",parent_path);
		return IMAX(dir->dev->info);
	}
	parent_dir->size++;
	dirent_s* entry=(dirent_s*)parent_dir->addr;
	entry+=parent_dir->size;
	strcpy(entry->name,target_name);
	// Allocate Disk Space
	entry->inum=_disk_inode_alloc(dir->dev);
	return entry->inum;
}
