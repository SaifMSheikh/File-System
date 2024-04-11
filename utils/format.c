#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE (4 * 1024)

// Disk Superblock (File Header)
typedef struct superblock_s {
    uint32_t size;        // Total Number Of Blocks
    uint32_t data_size;   // Number Of Data Blocks
    uint32_t inode_size;  // Number Of Inode Blocks
    uint32_t data_start;  // Data Starting Block
    uint32_t inode_start; // Inode Starting Block
} superblock_s;

int main() {
    // Create a superblock instance with sample values
    superblock_s superblock;
    superblock.size = 64;        // Total Number Of Blocks
    superblock.data_size = 56;    // Number Of Data Blocks
    superblock.inode_size = 5;   // Number Of Inode Blocks
    superblock.data_start = 8;     // Data Starting Block
    superblock.inode_start = 2;  // Inode Starting Block

    // Open or create a file for writing
    FILE* file = fopen("../data.dat", "w+");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Write the superblock structure to the beginning of the file
    size_t ret = fwrite(&superblock, sizeof(superblock_s), 1, file);
    if (ret != 1) {
        perror("Error writing superblock to file");
        fclose(file);
        return 1;
    }

    // Calculate the total size needed for padding
    long pad_size = superblock.size * BLOCK_SIZE - ftell(file);
    
    // Pad the file with zeroes to reach the desired length
    if (pad_size > 0) {
        char zero_buffer[BLOCK_SIZE];
        memset(zero_buffer, 0, BLOCK_SIZE);
        for (long i = 0; i < pad_size; i += BLOCK_SIZE) {
            ret = fwrite(zero_buffer, sizeof(char), BLOCK_SIZE, file);
            if (ret != BLOCK_SIZE) {
                perror("Error writing padding to file");
                fclose(file);
                return 1;
            }
        }
    }

    // Now, you can continue writing other data or close the file
    fclose(file);
    
    return 0;
}
