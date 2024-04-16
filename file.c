// #define NDEBUG
#include <string.h>
#include "inode.h"
#include "file.h"
#include "disk.h"
// Find Directory, Given Parent & Relative Path
uint16_t dir_lookup(const inode_s* dir,const char* target_path) {
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
		// Search Indirect Data
		dirent_s* entry=(dirent_s*)_disk_data_get(dir->dev,dir->info->addr[0]);
		for (int i=0;(i<dir->info->size)&&(i<NDIRENT);++i)
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
	// Search Indirect Data
	entry=(dirent_s*)_disk_data_get(dir->dev,dir->info->addr[0]);
	for(int i=0;(i<dir->info->size)&&(i<NDIRENT);++i)
		if (!strcmp(entry[i].name,target_path))
			return entry[i].inum;
	// Not Found
	return IMAX(dir->dev->info);
}
// Create Directory At Relative Path From Parent
inode_s dir_create(const inode_s* dir,const char* path) {
	inode_s node;
	node.valid=false;
	// Validate Input
	if (!dir->valid) {
		printf("Invalid Inode\n");
		return node;
	}
	// Check If Taken
	if (dir_lookup(dir,path)!=IMAX(dir->dev->info)) {
		printf("Filename Already Exists\n");
		return node;
	}
	// Get Parent Directory & Relative Path
	char parent_path[255]="";
	char target_name[DIRENT_NAME_LEN]="";
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
		return node;
	}
	// If Not Found, Use Current Directory As Parent
	inode_s parent_dir;
	if (!strcmp(parent_path,"")) {
		parent_dir=*dir;
		strcpy(target_name,path);
	}
	// Otherwise, Fetch Parent Directory
	else parent_dir=_inode_get(dir->dev,dir_lookup(dir,parent_path));
	if (!parent_dir.valid) {
		printf("Parent Directory Is Invalid");
		return node;
	}
	// Update Parent Directory Data
	if (parent_dir.info->size>=NDIRENT) {
		printf("Parent Directory (%s) Full\n",parent_path);
		return node;
	}
	// Update Indirect Data
	dirent_s* entry=(dirent_s*)_disk_data_get(dir->dev,parent_dir.info->addr[0]);
	strcpy(entry[parent_dir.info->size++].name,target_name);
	// Allocate Disk Space
	node=_inode_create(dir->dev,I_DIRE);
	if (!node.valid)
		return node;
	entry->inum=node.inum;
	// Add Parent Directory Reference
	entry=(dirent_s*)_disk_data_get(node.dev,node.info->addr[0]);
	strcpy(entry[0].name,"..");
	entry[0].inum=parent_dir.inum;
	strcpy(entry[1].name,".");
	entry[1].inum=node.inum;
	node.info->size+=2;
	return node;
}
void dir_print(const inode_s* dir) {
	// Validate Input
	if (!dir->valid||dir->info->type!=I_DIRE) {
		printf("Invalid Root Node\n");
		return;
	}
	// Print Entries
	inode_s   node;
	dirent_s* entry=(dirent_s*)_disk_data_get(dir->dev,dir->info->addr[0]);
	for (int i=0;(i<dir->info->size)&&(i<NDIRENT);++i) 
		printf("\n%s",entry[i].name);
	printf("\n");
}
bool dir_destroy(const inode_s* dir,const char* path) {
	// Get Target Directory
	inode_s target_dir=_inode_get(dir->dev,dir_lookup(dir,path));
	if (!target_dir.valid) { 
		printf("Failed To Fetch Target Directory\n");
		return false;
	}
	// Update Parent Directory
	inode_s parent_dir=_inode_get(dir->dev,dir_lookup(&target_dir,".."));
	if (!parent_dir.valid) {
		printf("Failed To Fetch Parent Directory\n");
		return false;
	}
	int i;
	dirent_s* entry=(dirent_s*)_disk_data_get(parent_dir.dev,parent_dir.info->addr[0]);
	for (i=0;i<parent_dir.info->size;++i) {
		if (entry[i].inum==target_dir.inum) {
			// Realign Array
			for (int j=i+1;j<parent_dir.info->size;++j)
				memmove(&entry[i],&entry[j],(parent_dir.info->size-j)*sizeof(dirent_s));
			parent_dir.info->size--;
			break;
		}
	}
	// Free Resources
	return _inode_destroy(&target_dir);
}
