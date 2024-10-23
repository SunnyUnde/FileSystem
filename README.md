# FileSystem

## Code is written in C language 

This code works on a virtual disk named as *"disk.teasage"*.
### Functanalities available in current code are:
* Store a file into disk
* Retrieve a file from disk
* List all files available on disk
* Delete a file form disk


### About the Code:
Disk has been divided mainly into two parts.  
The first part is approximately 12% of the disk size.This part is being used to store the meta data,the structures and the free block bit array.  
The structres conatins all the relevant data of the file stored in the disk.  
**Struct is as follow:**
```C  
struct File1  
{  
	char filename[48];  
	unsigned int start;  
	unsigned int size;  
	unsigned int last;  
	unsigned int next;  
}; 
``` 
1. _filename_ :stores the name of the file.
2. _start_ : stores the starting block number where the files's data is stored.
3. _size_ : stores number of continous block from the starting block.
4. _last_ : stores either the number of relevant data in the last block of the file or 0 which indicates that this is not the last structure of this file.
5. _next_ : stores either the index of the next file after this or the index of the next struct of the same file when the last==0

The first structure is being used to store the Disk information.  
*Free Bit* array has been implemented to know which blocks from the second part are free( 0 bit indicates the block is free and vice versa).

Automation has been implemented to check if file being copied to/from Disk and file being deleted is done properly or not.  
The result of this is being stored in the file named *'testResult'*.   
Here the 1 indicates that the two file has copied to/from Disk have same data and -1 indicates that the operation has not been done correctly.  
There is issue of deleted file as the information of all file being copied to/from the Disk is stored prior to its deletion from the virtual disk the checing of those files results in output being -1. This issue is yet to be handled in this code, reason for it being as checking or storing information of the deleted files will take either time or space.  

To check the deletion operation being performed correctly the file is searched in the disk. If not found the value 1 is being displayed in the file else -1 is displayed in the file.  

The blocksize of the virtual Disk must be stored in the file named 'diskdetailed'.(minimum block size must be size of struct File1,block size in the multiple of the size of struct File1 will help in reducing internal fragmentation).If the block size is changed then the previous disk with old blocksize must be deleted manually.  
*logfile* is created which stores all the latest information done on the disk.  
*testResult* contains the result of correctness of the information like copying into/from Disk and deletion of file in Disk.
### Main Subroutines:
* Stores File into Virtual Disk:  
	`vdcpto(char* filepath,char* filename)`:  
filepath: is file path of file on actual disk which is to be copied on the disk, filename: is the name to be used to stored the file.
It returns negative value on error and 1 for sucessfull storing the file.
* Copies File from Virtual Disk into Actual Disk.  
	`vdcpfrom(char* filepath,char* filename)`:  
filepath:is the path where the file has to be stored, filename:is the name of the file in virtual disk which is to be stored.
It returns negative value on occurence of error and 1 for successful completion of the subroutine.
* Delets File from Virtual Disk.  
	`DeleteFromDisk(char* filename)`:  
filename:is the name of the file to be deleted from the virtual disk.  
It returns negative value on occurence of error and 1 for successful completion of the subroutine.
* Displays File present on Virtual Disk.  
	`vdls(char* buff)`:  
buff: is the buffer in which the filename is to be stored. The buffers first 8 bytes must be reserved(First 4bytes indicates the index of the struct to be read and next 4 bytes indicates the size of the buffer).  
It returns the number of files read and negative value at occurence of Error.
