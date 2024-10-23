# FileSystem

## Overview
This code is written in C language and operates on a virtual disk named "disk.teasage".

## Available Functionality
* Store files into disk
* Retrieve files from disk
* List all files available on disk
* Delete files from disk

## Technical Details

### Disk Structure
The disk is divided into two main parts:
1. The first part (~12% of disk size) stores:
   - Metadata
   - File structures
   - Free block bit array
2. The second part stores the actual file data

### File Structure
Each file is represented by the following structure:
```C
struct File1 {
    char filename[48];
    unsigned int start;
    unsigned int size;
    unsigned int last;
    unsigned int next;
};
```

Structure fields:
- `filename`: Name of the file
- `start`: Starting block number where file data is stored
- `size`: Number of continuous blocks from the starting block
- `last`: Either contains the number of bytes in the last block, or 0 to indicate this is not the last structure of the file
- `next`: Either contains the index of the next file, or the index of the next structure of the same file (when last==0)

### Implementation Details
- The first structure stores disk information
- A Free Bit array tracks block availability (0 = free, 1 = occupied)
- Block size must be:
  - At least the size of `struct File1`
  - Preferably a multiple of `struct File1` size to reduce internal fragmentation

### System Files
- `diskdetailed`: Stores the block size configuration
- `logfile`: Records all recent disk operations
- `testResult`: Contains operation verification results
  - 1: Successful operation (files match or deletion confirmed)
  - -1: Failed operation

**Note**: When changing block size, the previous disk file must be manually deleted.

## Main Functions

### 1. Store File (Virtual Disk Copy To)
```C
int vdcpto(char* filepath, char* filename)
```
- `filepath`: Source file path on the actual disk
- `filename`: Desired name for storage in virtual disk
- Returns: 1 for success, negative value for error

### 2. Retrieve File (Virtual Disk Copy From)
```C
int vdcpfrom(char* filepath, char* filename)
```
- `filepath`: Destination path on actual disk
- `filename`: Name of file in virtual disk
- Returns: 1 for success, negative value for error

### 3. Delete File
```C
int DeleteFromDisk(char* filename)
```
- `filename`: Name of file to delete
- Returns: 1 for success, negative value for error

### 4. List Files
```C
int vdls(char* buff)
```
- `buff`: Buffer to store filenames
  - First 4 bytes: Index of structure to read
  - Next 4 bytes: Buffer size
- Returns: Number of files read or negative value for error

### Known Issues
The testing system stores file information before deletion operations. This causes deleted file checks to return -1 in the test results. This limitation exists to avoid additional time or space overhead in tracking deleted files.
