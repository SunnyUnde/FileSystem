//./a.out <input &>error >output
#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/types.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#define blksize (sizeof(long)*64)// block size is being defined as 512 bytes
#define ostream stdout
#define istream stdin
#define estream stderr

/*
 A struct has been defined to store the info of the continous block this will be storing: (filename)filename , (start)block number of the first block in the continous block,(size) total number of continous block,(last) the files last block may not take all the blksize(512 bytes) so the value of number of relevant bytes or if the value is 0 it represents that the variable next points to info of same file's next info ,(next) the next file's info.
*/

struct File1
{
	char filename[48];
	unsigned int start;
	unsigned int size;
	unsigned int last;
	unsigned int next;	
};
int total,SIZE,RES ;
/*
 This subroutine takes block number and reads 512 bytes of that particular block.
*/
int readDiskBlock(int fd,unsigned int blkno, char * buffptr)
{
	if(blkno>total)//blocknumber should not be greater than the total blocks in disk
		return -1;
	if(lseek(fd,blkno*blksize,SEEK_SET)==-1)
			return -1;
	return read(fd,buffptr,blksize);
}

/*
 This subroutine takes block number and writes 512 bytes to that particular block.
*/

int writeDiskBlock(int fd,unsigned int blkno,char * buffptr)
{
	if(blkno>total)//blocknumber should not be greater than the total blocks in disk
		return -1;
	if(lseek(fd,blkno*blksize,SEEK_SET)==-1)
		return -1;
	return (write(fd,buffptr,blksize));
}

/*
 This subroutine takes fd of file and tell number of blocks (of size 512 bytes each) required to store its data
*/

int blocksReq(int fd)
{
	long end = lseek(fd, 0, SEEK_END);
	if(end==-1)
		return -1;
	if(lseek(fd,0,SEEK_SET)==-1)
		return -1;
	int a=end/blksize;
	if(a==-1)
		return a;
	if(end%blksize==0)
		return a;
	else
		return a+1;
}

/*
 This subroutine takes a virtual disk fd and a block number to find a empty struct in that block (an empty struct is one with size ==0)
*/

int findStruct(int fd,unsigned int blk)
{
	if(blk>RES)
	{
		return -2;
	}
	struct File1 *arr=(struct File1*)malloc(sizeof(struct File1)*SIZE);
	if(arr==NULL)
		return -2;
	if(readDiskBlock(fd,blk/SIZE,(char*)arr)==-1)
		return -10;
	for(unsigned int i=blk%SIZE;i<SIZE;i++)
	{
		if(arr[i].size==0)// try using anything else than size
		{
			return i+blk;
		}
	}
	free(arr);
	return -1;
}

/*
This subroutine takes char* as argument and adds filename to it the char array's first 4 bytes will have value of block number to read with and next 4 bytes will have value of the size of the array.It returns number of filename stored int the array and stores the filename in the buf/.
*/

int vdls(char* buf)
{
	int fd=open("disk.teasage",O_RDONLY);
	if(fd==-1)
		return -12;
	int i=0;
	struct File1 *arr=(struct File1*)malloc(sizeof(struct File1)*SIZE);
	if(arr==NULL)
	{
		close(fd);
		return -1;
	}
	char *temp = (char*)malloc(sizeof(char)*48);
	if(temp==NULL)
	{
		close(fd);
		free(arr);
		return -1;
	}
	int* start = (int*)&buf[0];
	int * size = (int*)&buf[4];
	if(readDiskBlock(fd,(*start)/SIZE,(char*)arr)==-1)
	{
		close(fd);
		free(arr);
		free(temp);
		return -10;
	}
	if(*start==0)
	{
		if(arr[0].start==0)
		{
			close(fd);
			free(arr);
			free(temp);
			return 0;//no files in disk
		}
	}
	i=*start;
	int count,n;
	count = n = 0;
	while(n+8<*size-47&& arr[i%SIZE].next!=0)
	{
		i=arr[i%SIZE].next;
		if(readDiskBlock(fd,i/SIZE,(char*)arr)==-1)
		{
			close(fd);
			free(arr);
			free(temp);
			return -10;
		}
		strcpy(temp,arr[i%SIZE].filename);
		strcpy(buf+8+n,temp);
		if(arr[i%SIZE].last==0)
		{
			while(arr[i%SIZE].last==0)
			{
				i=arr[i%SIZE].next;
				if(readDiskBlock(fd,i/SIZE,(char*)arr)==-1)
				{
					close(fd);
					free(arr);
					free(temp);
					return -10;
				}
			}
		}
		n+=strlen(temp)+1;
		++count;
	}
	*start=i;
	close(fd);
	free(arr);
	free(temp);
	return count;
}

/*
 This subroutine takes virtual disk fd and value which represents the struct number which has to be free it then either adds the struct to the list of free blocks or free's the struct( struct member size equals to zero).
*/

int freeBlock(int fd,int index)
{
	struct File1* arr=(struct File1*)malloc(sizeof(struct File1)*SIZE);
	if(arr==NULL)
	{
		return -1;
	}
	if(readDiskBlock(fd,index/SIZE,(char*)arr)==-1)
	{
		return -10;
	}
	int start=arr[index%SIZE].start;
	int size=arr[index%SIZE].size;
	if(readDiskBlock(fd,0,(char*)arr)==-1)
	{
		return -10;
	}
	int p=1;
	int i=arr[1].next;
	while(start>arr[i%SIZE].start)
	{
		p=i;
		i=arr[i%SIZE].next;
		if(readDiskBlock(fd,i/SIZE,(char*)arr)==-1)
		{
			free(arr);
			return -10;
		}
	}
	int flag=0;
	if(start+size==arr[i%SIZE].start)
	{
		arr[i%SIZE].start=start;
		arr[i%SIZE].size+=size;
		flag=arr[i%SIZE].next;
		size=arr[i%SIZE].size;
		if(writeDiskBlock(fd,i/SIZE,(char*)arr)==-1)
		{
			free(arr);
			return -11;
		}
	}
	if(readDiskBlock(fd,p/SIZE,(char*)arr)==-1)
	{
		free(arr);
		return -10;
	}
	if(arr[p%SIZE].start+arr[p%SIZE].size==start)
	{
		arr[p%SIZE].size+=size;
		if(flag!=0)
		{
			arr[p%SIZE].next=flag;
		}
		if(writeDiskBlock(fd,i/SIZE,(char*)arr)==-1)
		{
			free(arr);
			return -11;
		}
		if(flag!=0)
		{
			if(readDiskBlock(fd,i/SIZE,(char*)arr)==-1)
			{
				free(arr);
				return -10;
			}
			arr[i%SIZE].size=0;
			if(writeDiskBlock(fd,i/SIZE,(char*)arr)==-1)
			{
				free(arr);
				return -10;
			}
		}
		else
		{
			flag=1;
		}
	}
	if(flag==0)
	{
		arr[p%SIZE].next=index;
		if(writeDiskBlock(fd,p/SIZE,(char*)arr)==-1)
		{
			free(arr);
			return -10;
		}
		if(readDiskBlock(fd,index/SIZE,(char*)arr)==-1)
		{
			free(arr);
			return -10;
		}
		arr[index%SIZE].next=i;
		if(writeDiskBlock(fd,index/SIZE,(char*)arr)==-1)
		{
			free(arr);
			return -10;
		}
	}
	else
	{
		if(readDiskBlock(fd,index/SIZE,(char*)arr)==-1)
		{
			free(arr);
			return -10;
		}
		arr[index%SIZE].size=0;
		if(writeDiskBlock(fd,index/SIZE,(char*)arr)==-1)
		{
			free(arr);
			return -10;
		}
	}
}

/*
 This subroutine takes filename as a argument and deletes that file from the virtual disk
*/
int deleteFileFromDisk(char *filename)
{
	int fd=open("disk.teasage",O_RDWR);
	if(fd==-1)
	{
		return -12;
	}
	struct File1* arr=(struct File1*)malloc(sizeof(struct File1)*SIZE);
	if(arr==NULL)
	{
		close(fd);
		return -1;
	}

	if(readDiskBlock(fd,0,(char*)arr)==-1)
	{
		close(fd);
		free(arr);
		return -10;
	}
	if(arr[0].start==0)
	{
		free(arr);
		return 0;
	}
	int i=arr[0].next;
	int n,p=0,j=0;
	for(n=arr[0].start;n>0;p=i,n--,i=arr[i%SIZE].next)//search for the file to be deleted
	{
		if(readDiskBlock(fd,i/SIZE,(char*)arr)==-1)
		{
			close(fd);
			free(arr);
			return -10;
		}
		if(strcmp(arr[i%SIZE].filename,filename)==0)
			break;
		j=i;
		while(arr[i%SIZE].last==0)
		{
			i=arr[i%SIZE].next;
			if(readDiskBlock(fd,i/SIZE,(char*)arr)==-1)
			{
				close(fd);
				free(arr);
				return -10;
			}
		}
	}
	if(n==0)
	{
		close(fd);
		free(arr);
		return -1;
	}
	int size=0,index=i;
	while(arr[i%SIZE].last==0)
	{
		size+=arr[i%SIZE].size;
		if(freeBlock(fd,i)<0)
		{
			close(fd);
			free(arr);
			return -1;
		}
		i=arr[i%SIZE].next;
		if(readDiskBlock(fd,i/SIZE,(char*)arr)==-1)
		{
			close(fd);
			free(arr);
			return -10;
		}
	}
	int next=arr[i%SIZE].next;
	size+=arr[i%SIZE].size;
	if(freeBlock(fd,i)<0)
	{
		close(fd);
		free(arr);
		return -1;
	}
	if(readDiskBlock(fd,p/SIZE,(char*)arr)==-1)
	{		
		close(fd);
		free(arr);
		return -10;
	}
	arr[p%SIZE].next=next;
	if(writeDiskBlock(fd,p/SIZE,(char*)arr)==-1)
	{
		close(fd);
		free(arr);
		return -11;
	}
	if(readDiskBlock(fd,0,(char*)arr)==-1)
	{
		close(fd);
		free(arr);
		return -10;
	}
	arr[0].start-=1;
	arr[0].size+=size;
	if(arr[0].last==index)
	{
		arr[0].last=p;
	}
	if(writeDiskBlock(fd,0,(char*)arr)==-1)
	{
		close(fd);
		free(arr);
		return -11;
	}
	return 1;
}

/*
This subroutine takes file path (file path of the file which is present in actual disk) and filename (file name of the file to be copied) and stores the file to the virtual disk
*/

int vdcpto(char* path,char *file)
{
	strcat(path,"/");
	strcat(path,file);
	int fd1=open(path,O_RDONLY);
	if(fd1==-1)
	{
		return -1;
	}
	int fd = open("disk.teasage",O_RDWR);
	if(fd==-1)
	{
		close(fd1);
		return -12;
	}
	int req=blocksReq(fd1);
	if(req==0)
	{
		return -1;// empty file can't be storesd
	}
	struct File1 *arr=(struct File1*)malloc(sizeof(struct File1)*SIZE);
	if(arr==NULL)
	{
		close(fd);
		close(fd1);
		return -1;
	}
	if(readDiskBlock(fd,0,(char*)arr)==-1)
	{
		free(arr);
		close(fd1);
		close(fd);
		return -1;
	}
	if(arr[0].start!=0)
	{
		char* buff=(char*)malloc(sizeof(char)*100);
		if(buff==NULL)
		{
			close(fd);
			close(fd1);
			return -1;
		}		
		int *start=(int*)&buff[0];
		*start=0;
		start=(int*)&buff[4];
		*start=100;
		int n;
		while((n=vdls(buff))>0)
		{
			for(int i=0,j=0;i<n;i++)
			{
				if(strcmp(buff+8+j,file)==0)
				{
					if((n=deleteFileFromDisk(buff+8+j))!=1)
					{
						free(arr);
						free(buff);
						close(fd);
						close(fd1);
						return n;
					}
					if(readDiskBlock(fd,0,(char*)arr)==-1)
					{
						free(buff);
						free(arr);
						close(fd1);
						close(fd);
						return -1;
					}
					break;
				}
				j+=strlen(buff+8+j)+1;
			}
		}
		free(buff);		
	}
	if(arr[0].size<req)//check if there is required free blocks in disk
	{
		free(arr);
		close(fd1);
		close(fd);
		return -2;
	}
	int rd,last,next,start,size,j,l,k=0;
	while((l=findStruct(fd,k))==-1)
	{
		if(l<-1)
		{
			free(arr);
			close(fd1);
			close(fd);
			return -13;
		}
		k+=SIZE;
		if(k>RES)
		{
			free(arr);
			close(fd1);
			close(fd);
			return -13;

		}
	}
	int index=l;
	k=l+1;
	int fre,p;
	int prev=arr[0].last;
	fre=arr[1].next;
	p=1;
	int r= req;
	char *buf=(char*)malloc(sizeof(char)*512);
	if(buf==NULL)
	{
		free(arr);
		close(fd1);
		close(fd);
		return -1;
	}
	while(req!=0)
	{
		if(fre==1)
		{
			close(fd);
			close(fd1);
			free(buf);
			free(arr);
			return -2;
		}
		else
		{
			if(readDiskBlock(fd,fre/SIZE,(char*)arr)==-1)
			{
				close(fd);
				close(fd1);
				free(buf);
				free(arr);
				return -10;
			}
		}
		start=arr[fre%SIZE].start;
		size=arr[fre%SIZE].size;
		for(j=0;j<size;j++,req--)
		{
			last=rd;
			if((rd=read(fd1,buf,blksize))==-1)
			{
				close(fd);
				close(fd1);
				free(buf);
				free(arr);
				return -3;
			}
			if(rd==0)
				break;
			if(writeDiskBlock(fd,start+j,buf)==-1)
			{
				close(fd);
				close(fd1);
				free(buf);
				free(arr);
				return -11;
			}
		}
		if(j==size && req==0)
		{
			last = rd;
			rd=0;
		}
		arr[fre%SIZE].start+=j;
		arr[fre%SIZE].size-=j;
		if(writeDiskBlock(fd,fre/SIZE,(char*)arr)==-1)
		{
			close(fd);
			close(fd1);
			free(buf);
			free(arr);
			return -11;
		}
		if(arr[fre%SIZE].size==0)
		{
			next=arr[fre%SIZE].next;
			fre=p;
			if(readDiskBlock(fd,fre/SIZE,(char*)arr)==-1)
			{
				close(fd);
				close(fd1);
				free(buf);
				free(arr);
				return -10;
			}
			arr[fre%SIZE].next=next;
			if(writeDiskBlock(fd,fre/SIZE,(char*)arr)==-1)
			{
				close(fd);
				close(fd1);
				free(buf);
				free(arr);
				return -11;
			}
		}
		p=fre;
		fre=arr[fre%SIZE].next;
		if(readDiskBlock(fd,l/SIZE,(char*)arr)==-1)
		{
			close(fd);
			close(fd1);
			free(buf);
			free(arr);
			return -10;
		}
		strcpy(arr[l%SIZE].filename,file);
		arr[l%SIZE].start=start;
		arr[l%SIZE].size=j;
		if(rd==0)
		{
			arr[l%SIZE].last=last;
			arr[l%SIZE].next=0;
		}
		else
		{
			arr[l%SIZE].last=0;
			while((arr[l%SIZE].next=findStruct(fd,k))==-1)
			{
				if(arr[l%SIZE].next<-1)
				{
					close(fd1);
					close(fd);
					free(buf);
					free(arr);
					return -13;
				}
				k+=(SIZE-(k%SIZE));
				if(k>RES)
				{
					close(fd1);
					close(fd);
					free(buf);
					free(arr);
					return -13;
				}
			}
		}
		fprintf(ostream,"\nid:%d\tFilename:%s\tStart:%d\tSize:%d\tLast:%d\tRd:%d",l,file,start,j,last,rd);
		next=arr[l%SIZE].next;
		if(writeDiskBlock(fd,l/SIZE,(char*)arr)==-1)
		{
			close(fd);
			close(fd1);
			free(buf);
			free(arr);
			return -11;
		}
		if(readDiskBlock(fd,prev/SIZE,(char*)arr)==-1)
		{
			close(fd);
			close(fd1);
			free(buf);
			free(arr);
			return -10;
		}
		arr[prev%SIZE].next=l;
		if(writeDiskBlock(fd,prev/SIZE,(char*)arr)==-1)
		{
			close(fd);
			close(fd1);
			free(buf);
			free(arr);
			return -11;
		}
		prev=l;
		l=next;
	}
	if(readDiskBlock(fd,0,(char*)arr)==-1)
	{
		close(fd);
		close(fd1);
		free(buf);
		free(arr);
		return -10;
	}
	arr[0].start+=1;
	arr[0].last=index;
	arr[0].size-=r;
	if(writeDiskBlock(fd,0,(char*)arr)==-1)
	{
		close(fd);
		close(fd1);
		free(buf);
		free(arr);
		return -11;
	}
	free(buf);
	free(arr);
	return 1;
}

/*
This subroutine takes file path(path where the file must be copied in actual disk) and filename(name of the file on virtual disk) and saves the file in actual disk at given path.
*/

int vdcpfrom(char* path,char *filename)
{
	int fd = open("disk.teasage",O_RDWR);
	if(fd==-1)
	{
		return -12;
	}
	struct File1 *arr=(struct File1*)malloc(sizeof(struct File1)*SIZE);
	if(arr==NULL)
	{
		close(fd);
		return -1;
	}
	if(readDiskBlock(fd,0,(char*)arr)==-1)
	{
		close(fd);
		free(arr);
		return -10;
	}
	if(arr[0].start==0)
	{
		close(fd);
		free(arr);
		return 0;
	}
	strcat(path,"/");
	strcat(path,filename);
	int n=arr[0].start;
	int i=arr[0].next;
	while(n!=0)
	{
		if(readDiskBlock(fd,i/SIZE,(char*)arr)==-1)
		{
			close(fd);
			free(arr);
			return -10;
		}
		if(strcmp(arr[i%SIZE].filename,filename)==0)
		{
			break;
		}
		i=arr[i%SIZE].next;
		n--;
	}
	if(n==0)
	{
		close(fd);
		free(arr);
		return -6;
	}
	int fd1=open(path, O_WRONLY | O_CREAT,00700);
	if(fd1==-1)
	{
		close(fd);
		free(arr);
		return -1;
	}
	char *buf=(char*)malloc(sizeof(char)*blksize);
	if(buf==NULL)
	{
		close(fd);
		close(fd1);
		free(arr);
		return -1;
	}
	while(1)
	{
		n=arr[i%SIZE].start;
		while(n<arr[i%SIZE].start+arr[i%SIZE].size-1)
		{
			if(readDiskBlock(fd,n,buf)==-1)
			{
				close(fd);
				close(fd1);
				free(arr);
				free(buf);
				return -10;
			}
			if(write(fd1,buf,blksize)==-1)
			{
				close(fd);
				close(fd1);
				free(arr);
				free(buf);
				return -4;
			}
			n++;
		}
		if(readDiskBlock(fd,n,buf)==-1)
		{
			close(fd);
			close(fd1);
			free(arr);
			free(buf);
			return -10;
		}
		if(arr[i%SIZE].last!=0)
		{
			int wd=write(fd1,buf,arr[i%SIZE].last);
			close(fd);
			close(fd1);
			free(arr);
			free(buf);
			if(wd==-1)
				return -4;
			break;
		}
		else
		{
			int wd=write(fd1,buf,blksize);
			if(wd==-1)
			{
				close(fd);
				close(fd1);
				free(buf);
				return -4;
			}
			i=arr[i%SIZE].next;
			if(readDiskBlock(fd,i/SIZE,(char*)arr)==-1)
			{
				close(fd);
				close(fd1);
				free(arr);
				free(buf);
				return -10;
			}
		}
	}
	//free(buf);
	return 1;
}
/*
This subroutine is used to create a file named "disk.teasage" of 10MB size.
*/
int createDisk()
{
	int fd = open("disk.teasage",O_WRONLY | O_CREAT,00700);
	long disksize= 10485760L - 1L;
	if(fd==-1)
		return -1;
	if (write(fd,"\0", 1) != 1)
		return -1;
    	if (lseek(fd, disksize, SEEK_SET) == -1)
		return -1;
    	if (write(fd, "\0", 1) != 1)	
		return -1;
	close(fd);
	return 1;
}
/*
 This subroutine prints the error number on estream
 -2 : No free space in disk 
 -3 :file read error
 -4 :file write error
 -6 :file not found on virtual disk
 -10 :Read Disk Error
 -11 :Write Disk Error
 -12 :Disk Open Error
 -13 :No Free struct
*/
void errorPrint(int e)
{
	fprintf(estream,"ERROR:%d\n",e);
}
/*
This subroutine is used to check the virtual disk if there are required blocks according to the need and also cacluated the value of SIZE(total number of struct in one block),total(total number of blocks in virtual disk) and RES(numner of reserved blocks to store the file info).
*/
int check()
{
	int fd = open("disk.teasage",O_RDWR);
	if(fd==-1)
	{
		return -12;
	}
	total=blocksReq(fd);	
	SIZE=sizeof(struct File1);
	SIZE= blksize/SIZE;
	struct File1* arr=(struct File1*)malloc(sizeof(struct File1)*SIZE);
	if(arr==NULL)
	{
		close(fd);
		return -1;
	}
	if(readDiskBlock(fd,0,(char*)arr)==-1)
	{
		return -10;
	}
	char * file=(char *)malloc(sizeof(char)*48);
	if(file==NULL)
	{
		close(fd);
		return -1;
	}
	RES = (total / (SIZE + 1) ); //+1 for taking max
	strcpy(file,"disk.tessage");
	if(strcmp(arr[0].filename,file)==0)
		return 1;
	strcpy(arr[0].filename,"disk.tessage");
	arr[0].size=total-RES;//no of free blocks
	arr[0].last=0;//last file index
	arr[0].next=0;//first file index
	arr[0].start=0;//no of files in the disk
	arr[1].size=1;
	arr[1].start=0;
	arr[1].next=2;
	arr[1].last=2;
	arr[2].start=RES;//start of free block
	arr[2].size=total-RES;//size of free block
	arr[2].next=1;//next free block info
	//arr[2].last=1;//prev free block info
	for(int j=3;j<SIZE;j++)
	{
		arr[j].size=0;			
	}
	if(writeDiskBlock(fd,0,(char*)arr)==-1)
		return -11;
	int i=1;
	for(int j=0;j<SIZE;j++)
	{
		arr[j].size=0;			
	}
	while(i<RES)
	{
		if(writeDiskBlock(fd,i,(char*)arr)==-1)
			return -11;
		i++;
	}
	close(fd);
	return 1;
}

int main()
{
	int ch,e;
	if(check()<0)
	{
		if((e=createDisk())!=1)//remove it later
		{
			errorPrint(e);
		}
		if((e=check())<0)
			errorPrint(e);
	}
	char * path= (char*)malloc(sizeof(char)*50);
	char * file = (char *)malloc(sizeof(char)*48);
	char *buf= (char*)malloc(sizeof(char)*100);
	int * start =(int*)&buf[0];
	*start=0;
	start = (int*)&buf[4];
	*start=100;
	if(buf==NULL || path == NULL || file ==NULL)
	{
		return -1;
	}
	do
	{
		fprintf(ostream,"\n1.List all the files\n2.Copy to VirtuaL Disk\n3.Copy from Virutal Disk\n4.Delete a File\n0.Exit\nEnter your choice\t");
		fscanf(istream,"%d",&ch);
		switch(ch)
		{
			case 1:				
				while((e=vdls(buf))>0)
				{
					for(int i=0,j=0;i<e;i++)
					{
						fprintf(ostream,"%s\n",buf+8+j);
						j+=strlen(buf+8+j)+1;
					}
				}
				if(e<0)
				{
					errorPrint(e);
				}
				else
				{
					start=(int*)&buf[0];
					*start=0;
				}
				break;
			case 2:
				fprintf(ostream,"\nEnter the path\t");
				fscanf(istream," %[^\n]",path);
				fprintf(ostream,"Enter the filename\t");
				fscanf(istream," %[^\n]",file);
				if((e=vdcpto(path,file))<=0)
				{
					errorPrint(e);
				}
				break;
			case 3:
				fprintf(ostream,"\nEnter the path\t");
				fscanf(istream," %[^\n]",path);
				fprintf(ostream,"Enter the filename\t");
				fscanf(istream," %[^\n]",file);
				if((e=vdcpfrom(path,file))<=0)
				{
					errorPrint(e);
				}
				break;
			case 4:
				fprintf(ostream,"Enter a filename\t");
				fscanf(istream," %[^\n]",file);
				if((e=deleteFileFromDisk(file))!=1)
					errorPrint(e);
				break;
			case 0:
				fprintf(ostream,"Exit\n");
				break;
			default:
				fprintf(ostream,"INVALID CHOICE\n");
		}
	}while(ch!=0);
	return 1;
}
