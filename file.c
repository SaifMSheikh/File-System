// #define NDEBUG
#include <string.h>
#include "inode.h"
#include "file.h"
#include "disk.h"
// Find Directory, Given Parent & Relative Path
uint16_t dir_lookup(const inode_s* dir,const char* target_path) {
	// Validate Inputs
	if (!dir->valid||dir->info->type!=I_DIRE) {
		printf("Invalid Root Inode\n");
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
// Print Directory Entries (Non-Recursive)
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
// Create Directory At Relative Path From Parent
uint16_t file_create(const inode_s* dir,const char* path,const uint8_t type) {
	inode_s node;
	node.valid=false;
	// Validate Input
	if (!dir->valid) {
		printf("Invalid Inode\n");
		return IMAX(dir->dev->info);
	}
	// Check If Taken
	if (dir_lookup(dir,path)!=IMAX(dir->dev->info)) {
		printf("Filename Already Exists\n");
		return IMAX(dir->dev->info);
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
		return IMAX(dir->dev->info);
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
		return IMAX(dir->dev->info);
	}
	// Update Parent Directory Data
	if (parent_dir.info->size>=NDIRENT) {
		printf("Parent Directory (%s) Full\n",parent_path);
		return IMAX(dir->dev->info);
	}
	dirent_s* entry=(dirent_s*)_disk_data_get(dir->dev,parent_dir.info->addr[0]);
	strcpy(entry[parent_dir.info->size++].name,target_name);
	// Allocate Disk Space
	node=_inode_create(dir->dev,type);
	if (!node.valid)
		return IMAX(dir->dev->info);
	entry->inum=node.inum;
	if (node.info->type==I_DIRE) {
		// Add Parent & Self Directory Entries
		dirent_s* entry=(dirent_s*)_disk_data_get(node.dev,node.info->addr[0]);
		strcpy(entry[0].name,"..");
		entry[0].inum=parent_dir.inum;
		strcpy(entry[1].name,".");
		entry[1].inum=node.inum;
		node.info->size+=2;
	}
	return node.inum;
}
bool file_delete(const inode_s* dir,const char* path) {
	// Get Target
	inode_s target=_inode_get(dir->dev,dir_lookup(dir,path));
	if (!target.valid) { 
		printf("Failed To Fetch Target Directory\n");
		return false;
	}
	if (target.info->ref_count) {
		printf("File Cannot Be Deleted While Open\n");
		return false;
	}
	// Update Parent Directory
	inode_s parent_dir;
	if (target.info->type==I_DIRE)
		{ parent_dir=_inode_get(dir->dev,dir_lookup(&target,"..")); }
	else {
		// Get Parent Directory Path
		char parent_path[255]="";
		for(int i=strlen(path)-1;i>=0;--i) {
			if (path[i]=='/') {
				strncpy(parent_path,path,i);
				break;
			}
		}
		// If Not Found, Use Current Directory As Parent
		if (!strcmp(parent_path,""))
			parent_dir=*dir;
		// Otherwise, Fetch Parent Directory
		else parent_dir=_inode_get(dir->dev,dir_lookup(dir,parent_path));
	}
	if (!parent_dir.valid) {
		printf("Failed To Fetch Parent Directory\n");
		return false;
	}
	int i;
	dirent_s* entry=(dirent_s*)_disk_data_get(parent_dir.dev,parent_dir.info->addr[0]);
	for (i=0;i<parent_dir.info->size;++i) {
		if (entry[i].inum==target.inum) {
			// Realign Array
			for (int j=i+1;j<parent_dir.info->size;++j)
				memmove(&entry[i],&entry[j],(parent_dir.info->size-j)*sizeof(dirent_s));
			parent_dir.info->size--;
			break;
		}
	}
	// Free Resources
	return _inode_destroy(&target);
}
// Get File Handle, Given Directory & Relative Path
file_s file_open(const inode_s* dir,const char* path,const uint8_t mode) {
	file_s file;
	file.valid=false;
	// Validate Inputs & Get Target Inode
	file.node=_inode_get(dir->dev,dir_lookup(dir,path));
	if (!file.node.valid||file.node.info->type==I_DIRE) {
		printf("Invalid Target\n");
		return file;
	} 
	// Populate File Handle
	file.iter=0;
	file.mode=mode;
	// Increment File Reference Count
	file.node.info->ref_count++;
	file.valid=true;
	return file;
}
// Release File Handle
bool file_close(file_s* file) {
	// Validate Input
	if (!file->valid) {
		printf("Invalid File Handle\n");
		return false;
	}
	file->valid=false;
	// Release Inode Reference
	file->node.info->ref_count--;
	return true;
}
// Write n Bytes To File
uint32_t file_write(file_s* file,uint8_t* buffer,const size_t n_bytes) {
	// Validate Inputs
	if (!file->valid||!(file->mode&FILE_WRITABLE_BIT)) {
		printf("Invalid File Handle\n");
		return file->iter;
	}
	if (file->iter+n_bytes>=BLOCK_SIZE*(NDIRECT+NINDIRECT)) {
		printf("Cannot Exceed File Size %ld",BLOCK_SIZE*(NDIRECT+NINDIRECT));
		return file->iter;
	}
	// Get Data At Iterator
	uint8_t* data;
	// 1 - Check Direct Data Section
	if (file->iter<BLOCK_SIZE*NDIRECT) 
		data=_disk_data_get(file->node.dev,file->node.info->addr[file->iter/BLOCK_SIZE]);
	// 2 - Check Indirect Data Section
	else {
		uint16_t* entry=(uint16_t*)_disk_data_get(file->node.dev,file->node.info->addr[NDIRECT]);
		int block_no=(file->iter/BLOCK_SIZE)-NDIRECT;
		data=_disk_data_get(file->node.dev,file->node.info->addr[block_no]);
	}
	// 3 - Get Offset Into Block
	data+=(file->iter%BLOCK_SIZE);
	// Write Data From Buffer & Increment Iterator
	memcpy(data,buffer,n_bytes);
	file->iter+=n_bytes;
	return file->iter;
}
// Read n Bytes From File
uint32_t file_read(file_s* file,uint8_t* buffer,size_t n_bytes) {
	// Validate Inputs
	if (!file->valid||!(file->mode&FILE_READABLE_BIT)) {
		printf("Invalid File Handle\n");
		return 0;
	}
	// Get Data At Iterator
	uint8_t* data;
	// 1 - Check Direct Data Section
	if (file->iter<BLOCK_SIZE*NDIRECT) 
		data=_disk_data_get(file->node.dev,file->node.info->addr[file->iter/BLOCK_SIZE]);
	// 2 - Check Indirect Data Section
	else {
		uint16_t* entry=(uint16_t*)_disk_data_get(file->node.dev,file->node.info->addr[NDIRECT]);
		int block_no=(file->iter/BLOCK_SIZE)-NDIRECT;
		data=_disk_data_get(file->node.dev,file->node.info->addr[block_no]);
	}
	// 3 - Get Offset Into Block
	data+=(file->iter%BLOCK_SIZE);
	// Write Data Into Buffer & Increment Iterator
	memcpy(buffer,data,n_bytes);
	file->iter+=n_bytes;
	return file->iter;
}
// Set File Iterator Position
bool file_seek(file_s* file,const uint32_t position) {
	// Validate Inputs
	if (!file->valid) {
		printf("Invalid File Handle\n");
		return false;
	}
	if (position<0||position>=BLOCK_SIZE*(NDIRECT+NINDIRECT)) {
		printf("Invalidd Iterator Position\n");
		return false;
	}
	// Set Iterator Position
	file->iter=position;
	return true;
}
// Get File Iterator Position
uint32_t file_tell(const file_s* file) {
	// Validate Input
	if (!file->valid) {
		printf("Invalid File Handle\n");
		return false;
	}
	// Get Iterator Position
	return file->iter;
}
