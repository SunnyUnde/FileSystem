#include<stdio.h>
#include<stdbool.h>
#include<math.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/types.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<stddef.h>
#include "myalloc.h"
#define ostream stdout
#define istream stdin
#define estream stderr
#define SetBit(A,k)     ( A[(k/8)] |= (1 << (k%8)) )
#define ClearBit(A,k)   ( A[(k/8)] &= ~(1 << (k%8)) )
#define TestBit(A,k)    ( A[(k/8)] & (1 << (k%8)) )

struct File1
{
	char filename[48];
	unsigned int start;
	unsigned int size;
	unsigned int last;
	unsigned int next;
};
int TOTAL,SIZE,RES,blksize,TSTRUCT;

/*
Lseek (Disk) Error	-2
Lseek (File) Error	-3
Read (Disk) Error	-4
Read (File) Error	-5
Write (Disk) Error	-6
Write (File) Error 	-7
Malloc Allocation Error	-8
No Free Struct Error	-9
No Free Block Error	-10
Access Beyond Total	-11
Open Disk Error		-12
Open File Error		-13
Create Disk Error	-14
Create File Error 	-15
Block Size Invalid 	-16
Empty File 		-17
Duplicate File 		-18
File not Found 		-19
Check() Error		-20
Rquired Blocks>Free Blocks -21
*/

int readDiskBlock(int fd,unsigned int blkno, char * buffptr)
{
	if(blkno>TOTAL)//blocknumber should not be greater than the total blocks in disk
		return -11;
	if(lseek(fd,blkno*blksize,SEEK_SET)==-1)
		return -2;
	return read(fd,buffptr,blksize);
}

/*
   This subroutine takes block number and writes 512 bytes to that particular block.
*/

int writeDiskBlock(int fd,unsigned int blkno,char * buffptr)
{
	if(blkno>TOTAL)//blocknumber should not be greater than the total blocks in disk
		return -11;
	if(lseek(fd,blkno*blksize,SEEK_SET)==-1)
		return -2;
	return (write(fd,buffptr,blksize));
}

/*
Retuen total blocks required of the blocksize for particular file
*/

int blocksReq(int fd)
{
	long end = lseek(fd, 0, SEEK_END);
	if(end==-1)
		return -3;
	if(lseek(fd,0,SEEK_SET)==-1)
		return -3;
	int a=end/blksize;
	if(a==-1)
		return a;
	if(end%blksize==0)
		return a;
	else
		return a+1;
}

/*
	Returns bool value about given character numeric or not
*/

bool isNumericChar(char x)
{
	return (x >= '0' && x <= '9') ? true : false;
}

/*
 This subroutines is similar to Atoi(ASCII value to integer)
*/

int myAtoi(char* str)
{
	if (*str == '\0')
		return 0;
	int res = 0;
	int sign = 1;
	int i = 0;
	if (str[0] == '-')
	{
		sign = -1;
		i++;
	}
	for (; str[i] != '\0'; ++i)
	{
		if (isNumericChar(str[i]) == false)
			return 0;
		res = res * 10 + str[i] - '0';
	}
	return sign * res;
}

/*
	This subroutine resets the disk according the blocksize
*/

int reset(int fd)
{
	if((TOTAL = blocksReq(fd))<0)
	{
		return TOTAL;
	}
	TSTRUCT= ceil((float)TOTAL* (0.12));//12% of the disk size would be used for total meta data
	int BSIZE = ceil((float)(TOTAL-TSTRUCT) /(float)(blksize*sizeof(unsigned char)*8));
	RES =TSTRUCT;
	TSTRUCT-=BSIZE;
	SIZE = sizeof(struct File1);
	SIZE = blksize/SIZE;
	struct File1* arr=(struct File1*)mymalloc(sizeof(struct File1)*SIZE);
	if(arr==NULL)
	{
		return -8;
	}
	strcpy(arr[0].filename,"disk.teasage");
	arr[0].size=TOTAL-RES;//no of myfree blocks
	arr[0].last=0;//last file index
	arr[0].next=0;//first file index
	arr[0].start=0;//total files in the disk
	if(writeDiskBlock(fd,0,(char*)arr)<0)
	{
		myfree(arr);
		return -6;
	}
	int i;
	for(i=1;i<SIZE;i++)
	{
		arr[i].size=0;
	}
	if(writeDiskBlock(fd,0,(char*)arr)<0)
	{
		myfree(arr);
		return -6;
	}
	arr[0].size=0;
	for(i=1;i<TSTRUCT;i++)
	{
		if(writeDiskBlock(fd,i,(char*)arr)<0)
		{
			myfree(arr);
			return -6;
		}
	}
	myfree(arr);
	unsigned char* a=(unsigned char*)mymalloc(sizeof(unsigned char)*blksize);
	if(a==NULL)
	{
		return -8;
	}
	for(i=0;i<blksize*sizeof(unsigned char);i++)
	{
		a[i]^=a[i];
	}
	for(i=TSTRUCT;i<RES;i++)
	{
		if(writeDiskBlock(fd,i,(char*)a)<0)
		{
			myfree(a);
			return -6;
		}
	}
	myfree(a);
	return 1;
}

/*
This subroutine is used to create a vritual disk of the provided blocksize
*/

int createDisk()
{
	long disksize=10485760L - 1L;
	int fd= open("disk.teasage",O_RDONLY);
	if(fd!=-1)
	{
		close(fd);
		return 1;//disk is already present but size is yet to be checked
	}	
	fd= open("disk.teasage",O_WRONLY | O_CREAT, 00700);
	if(fd==-1)
	{
		return -12;//disk can't be created
	}
	if (write(fd,"\0", 1) != 1)
	{
		close(fd);
		return -6;//write Error on Disk
	}	
	if (lseek(fd, disksize, SEEK_SET) == -1)
	{
		close(fd);
		return -2;//lseek Error on Disk
	}
	if (write(fd, "\0", 1) != 1)
	{
		close(fd);
		return -6;//write Error on Disk
	}
	int fd1= open("diskdetailed",O_RDWR);
	if(fd1==-1)
	{
		close(fd);
		return -13;
	}
	char* buf=(char*)mymalloc(sizeof(char)*100);
	if(buf==NULL)
	{
		close(fd);
		close(fd1);
		return -8;//no memory to allocate
	}
	char ch;
	int rd,i=0;
	while((rd=read(fd1,&ch,1))!=0 && i<100)
	{
		if(rd==-1)
		{
			close(fd);
			close(fd1);
			myfree(buf);
			return -5;
		}
		if(ch=='\n')
		{
			buf[i]='\0';
			close(fd1);
			break;
		}
		buf[i]=ch;
		++i;
	}
	blksize=myAtoi(buf);//Block size is should be the multiple of sizeof(struct File1)
	myfree(buf);	
	if(blksize<sizeof(struct File1))
	{
		close(fd);
		return -1;
	}	
	if(blksize==0)
	{
		close(fd1);
		return -1;
	}
	if(reset(fd)<0)
	{
		close(fd);
		return -1;
	}
	close(fd);
	return 1;
}

/*
	This suboroutine checks if the disk present is valid or not
*/

int check()
{
	int fd= open("disk.teasage",O_RDWR);
	if(fd==-1)
		return -12;// Disk not Found
	int fd1= open("diskdetailed",O_RDONLY);
	if(fd1==-1)
	{
		close(fd);
		return -13;
	}
	char* buff=(char*)mymalloc(sizeof(char)*100);
	if(buff==NULL)
	{
		close(fd);
		close(fd1);
		return -8;
	}
	char ch;
	int rd,i=0;
	while((rd=read(fd1,&ch,1))!=0 && i<100)
	{
		if(rd==-1)
		{
			close(fd);
			close(fd1);
			myfree(buff);
			return -5;
		}
		if(ch=='\n')
		{
			buff[i]='\0';
			close(fd1);
			break;
		}
		buff[i]=ch;
		++i;
	}
	blksize=myAtoi(buff);//Block size is should be the multiple of sizeof(struct File1)
	myfree(buff);		
	if(blksize<sizeof(struct File1))
	{
		close(fd);
		return -1;
	}
	if(blksize==0)
	{
		close(fd);
		return -1;
	}
	if((TOTAL = blocksReq(fd))<0)
	{
		close(fd);
		return TOTAL;
	}
	TSTRUCT= ceil((float)TOTAL* (0.12));//12% of the disk size would be used for total meta data
	int BSIZE = ceil((float)(TOTAL-TSTRUCT) /(float)(blksize*sizeof(unsigned char)*8));
	RES =TSTRUCT;
	TSTRUCT-=BSIZE;
	SIZE = sizeof(struct File1);
	SIZE = blksize/SIZE;
	struct File1* arr=(struct File1*)mymalloc(sizeof(struct File1)*SIZE);
	if(arr==NULL)
	{
		close(fd);
		return -8;
	}
	if(readDiskBlock(fd,0,(char*)arr)<0)
	{
		myfree(arr);
		return -4;
	}
	if(strcmp(arr[0].filename,"disk.teasage")!=0)
	{
		if(reset(fd)<0)
		{
			myfree(arr);
			close(fd);
			return -1;
		}
	}
	myfree(arr);
	close(fd);
	return 1;
}
/*
	This subroutine is used to find a myfree struct 
*/
int findStruct(int fd,unsigned int blk)
{
	
	if(blk>=TSTRUCT)
	{
		return -2;
	}
	struct File1 *arr=(struct File1*)mymalloc(sizeof(struct File1)*SIZE);
	if(arr==NULL)
		return -8;
	if(readDiskBlock(fd,blk/SIZE,(char*)arr)<0)
	{
		myfree(arr);
		return -4;
	}
	for(unsigned int i=blk%SIZE;i<SIZE;i++)
	{
		if(arr[i].size==0)// try using anything else than size
		{
			myfree(arr);
			return i+(blk/SIZE)*SIZE;
		}
	}
	myfree(arr);
	return -1;
}

/*
	This subroutine is used to find Free Block in the Disk.
*/

int findFree(int fd,int blk,int s)
{
	if(blk >=RES || blk<TSTRUCT)
	{
		return -2;
	}
	unsigned char* arr= (unsigned char*)mymalloc(sizeof(unsigned char)*blksize);
	if(arr==NULL)
	{
		return -8;
	}
	if(readDiskBlock(fd,blk,(char*)arr)<0)
	{
		myfree(arr);
		return -4;
	}
	blk-=TSTRUCT;
	for(int i=s;i<(sizeof(unsigned char)*blksize*8);i++)
	{
		if(!TestBit(arr,i))
		{
			myfree(arr);
			return i+((blk)*sizeof(unsigned char)*8*blksize)+RES;
		}
	}
	myfree(arr);
	return -1;
}

/*
	As a bit array has been used to maintain the myfree blocks this subroutine is used to set the bit of the particular block
*/

int setFreeB(int fd,int blk,int fre)
{
	if(blk >=RES || blk<TSTRUCT)
	{
		return -2;
	}
	unsigned char* arr= (unsigned char*)mymalloc(sizeof(unsigned char)*blksize);
	if(arr==NULL)
	{
		return -8;
	}
	if(readDiskBlock(fd,blk,(char*)arr)<0)
	{
		myfree(arr);
		return -4;
	}
	SetBit(arr,fre);
	if(writeDiskBlock(fd,blk,(char*)arr)<0)
	{
		myfree(arr);
		return -6;
	}
	myfree(arr);
	return 1;
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
	struct File1 *arr=(struct File1*)mymalloc(sizeof(struct File1)*SIZE);
	if(arr==NULL)
	{
		close(fd);
		return -8;
	}
	char* temp = (char*)mymalloc(sizeof(char)*48);
	if(temp==NULL)
	{
		close(fd);
		myfree(arr);
		return -8;
	}
	int* start = (int*)&buf[0];
	int* size = (int*)&buf[4];
	if(readDiskBlock(fd,(*start)/SIZE,(char*)arr)<0)
	{
		close(fd);
		myfree(arr);
		myfree(temp);
		return -4;
	}
	if(*start==0)
	{
		if(arr[0].start==0)
		{
			close(fd);
			myfree(arr);
			myfree(temp);
			return 0;//no files in disk
		}
	}
	i=*start;
	int count,n;
	count = n = 0;
	while(n+8<*size-48&& arr[i%SIZE].next!=0)
	{
		i=arr[i%SIZE].next;
		if(readDiskBlock(fd,i/SIZE,(char*)arr)<0)
		{
			close(fd);
			myfree(arr);
			myfree(temp);
			return -4;
		}
		strcpy(temp,arr[i%SIZE].filename);
		strcpy(buf+8+n,temp);
		if(arr[i%SIZE].last==0)
		{
			while(arr[i%SIZE].last==0)
			{
				i=arr[i%SIZE].next;
				if(readDiskBlock(fd,i/SIZE,(char*)arr)<0)
				{
					close(fd);
					myfree(arr);
					myfree(temp);
					return -4;
				}
			}
		}
		n+=strlen(temp)+1;
		++count;
	}
	*start=i;
	close(fd);
	myfree(arr);
	myfree(temp);
	return count;
}


/*
	This subroutine is used to store a file from given path as the filename provided into the disk.
*/

int vdcpto(char* path,char *file)
{
	int fd1=open(path,O_RDONLY);
	if(fd1==-1)
	{
		return -13;
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
		return -17;// empty file can't be storesd
	}
	struct File1 *arr=(struct File1*)mymalloc(sizeof(struct File1)*SIZE);
	if(arr==NULL)
	{
		close(fd);
		close(fd1);
		return -8;
	}
	if(readDiskBlock(fd,0,(char*)arr)<0)
	{
		myfree(arr);
		close(fd1);
		close(fd);
		return -4;
	}
	//checking if file with similar name exist already or not
	if(arr[0].start!=0)
	{
		char* buff=(char*)mymalloc(sizeof(char)*100);
		if(buff==NULL)
		{
			close(fd);
			close(fd1);
			myfree(arr);
			return -8;
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
					myfree(arr);
					myfree(buff);
					close(fd);
					close(fd1);
					return -18;//duplicate file found	
				}
				j+=strlen(buff+8+j)+1;
			}
		}
		myfree(buff);		
	}
	if(arr[0].size<req)//check if there is required myfree blocks in disk
	{
		myfree(arr);
		close(fd1);
		close(fd);
		return -21;
	}
	int rd,last,next,start,size,j,l,k=0;
	// Findinf the myfree struct to store the file info
	while((l=findStruct(fd,k))<0)
	{
		if(l<-1)
		{
			myfree(arr);
			close(fd1);
			close(fd);
			return l;
		}
		k+=SIZE;
		if(k>RES)
		{
			myfree(arr);
			close(fd1);
			close(fd);
			return -9;

		}
	}
	int index=l;
	k=l+1;
	int fre,p;
	int prev=arr[0].last;
	//fre=arr[1].next;
	//p=1;
	int r= req;
	char *buf=(char*)mymalloc(sizeof(char)*blksize);
	if(buf==NULL)
	{
		myfree(arr);
		close(fd1);
		close(fd);
		return -8;
	}
	int freeb=TSTRUCT;
	while((fre=findFree(fd,freeb,0))<0)
	{
		if(fre<-1)
		{
			myfree(buf);
			myfree(arr);
			close(fd1);
			close(fd);
			return fre;
		}
		freeb++;
	}
	p=fre;
	while(req!=0)
	{
		start = fre;
		size=0;
		while(1)
		{
			last=rd;
			if((rd=read(fd1,buf,blksize))<0)
			{
				close(fd);
				close(fd1);
				myfree(buf);
				myfree(arr);
				return -5;
			}
			if(rd==0)
				break;
			if(writeDiskBlock(fd,fre,buf)<0)
			{
				close(fd);
				close(fd1);
				myfree(buf);
				myfree(arr);
				return -6;
			}
			size++;
			req--;
			p=fre;
			if(setFreeB(fd,freeb,(fre)-((freeb-TSTRUCT)*sizeof(unsigned char)*8*blksize)-RES)<0)
			{
				close(fd);
				close(fd1);
				myfree(buf);
				myfree(arr);
				return -2;
			}
			while((fre=findFree(fd,freeb,(fre+1)-((freeb-TSTRUCT)*sizeof(unsigned char)*8*blksize)-RES))<0)
			{
				if(fre<-1)
				{
					myfree(buf);
					myfree(arr);
					close(fd1);
					close(fd);
					return fre;
				}
				freeb++;
				fre=((sizeof(unsigned char)*8*blksize)*(freeb-TSTRUCT))+RES-1;
			}
			if(fre!=p+1)
			{
				break;
			}
		}
		if(readDiskBlock(fd,l/SIZE,(char*)arr)<0)
		{
			close(fd);
			close(fd1);
			myfree(buf);
			myfree(arr);
			return -4;
		}
		strcpy(arr[l%SIZE].filename,file);
		arr[l%SIZE].start=start;
		arr[l%SIZE].size=size;
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
					myfree(buf);
					myfree(arr);
					return arr[l%SIZE].next;
				}
				k+=(SIZE-(k%SIZE));
				if(k>RES)
				{
					close(fd1);
					close(fd);
					myfree(buf);
					myfree(arr);
					return -9;
				}
			}
			k=arr[l%SIZE].next+1;
		}
		next=arr[l%SIZE].next;
		if(writeDiskBlock(fd,l/SIZE,(char*)arr)<0)
		{
			close(fd);
			close(fd1);
			myfree(buf);
			myfree(arr);
			return -6;
		}
		if(readDiskBlock(fd,prev/SIZE,(char*)arr)<0)
		{
			close(fd);
			close(fd1);
			myfree(buf);
			myfree(arr);
			return -4;
		}
		arr[prev%SIZE].next=l;
		if(writeDiskBlock(fd,prev/SIZE,(char*)arr)<0)
		{
			close(fd);
			close(fd1);
			myfree(buf);
			myfree(arr);
			return -6;
		}
		prev=l;
		l=next;
	}
	if(readDiskBlock(fd,0,(char*)arr)<0)
	{
		close(fd);
		close(fd1);
		myfree(buf);
		myfree(arr);
		return -4;
	}
	arr[0].start+=1;
	arr[0].last=index;
	arr[0].size-=r;
	if(writeDiskBlock(fd,0,(char*)arr)<0)
	{
		close(fd);
		close(fd1);
		myfree(buf);
		myfree(arr);
		return -6;
	}
	myfree(buf);
	myfree(arr);
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
	struct File1 *arr=(struct File1*)mymalloc(sizeof(struct File1)*SIZE);
	if(arr==NULL)
	{
		close(fd);
		return -8;
	}
	if(readDiskBlock(fd,0,(char*)arr)<0)
	{
		close(fd);
		myfree(arr);
		return -4;
	}
	if(arr[0].start==0)
	{
		close(fd);
		myfree(arr);
		return 0;
	}
	int n=arr[0].start;
	int i=arr[0].next;
	while(n!=0)
	{
		if(readDiskBlock(fd,i/SIZE,(char*)arr)<0)
		{
			close(fd);
			myfree(arr);
			return -4;
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
		myfree(arr);
		return -6;
	}
	int fd1=open(path, O_WRONLY | O_CREAT,00700);
	if(fd1==-1)
	{
		close(fd);
		myfree(arr);
		return -13;
	}
	char *buf=(char*)mymalloc(sizeof(char)*blksize);
	if(buf==NULL)
	{
		close(fd);
		close(fd1);
		myfree(arr);
		return -8;
	}
	while(1)
	{
		n=arr[i%SIZE].start;
		while(n<arr[i%SIZE].start+arr[i%SIZE].size-1)
		{
			if(readDiskBlock(fd,n,buf)<0)
			{
				close(fd);
				close(fd1);
				myfree(arr);
				myfree(buf);
				return -4;
			}
			if(write(fd1,buf,blksize)<0)
			{
				close(fd);
				close(fd1);
				myfree(arr);
				myfree(buf);
				return -7;
			}
			n++;
		}
		if(readDiskBlock(fd,n,buf)<0)
		{
			close(fd);
			close(fd1);
			myfree(arr);
			myfree(buf);
			return -4;
		}
		if(arr[i%SIZE].last!=0)
		{
			int wd=write(fd1,buf,arr[i%SIZE].last);
			if(wd==-1)
			{
				close(fd);
				close(fd1);
				myfree(arr);
				myfree(buf);
				return -7;
			}
			break;
		}
		else
		{
			int wd=write(fd1,buf,blksize);
			if(wd==-1)
			{
				close(fd);
				close(fd1);
				myfree(arr);
				myfree(buf);
				return -7;
			}
			i=arr[i%SIZE].next;
			if(readDiskBlock(fd,i/SIZE,(char*)arr)<0)
			{
				close(fd);
				close(fd1);
				myfree(arr);
				myfree(buf);
				return -4;
			}
		}
	}
	close(fd);
	close(fd1);
	myfree(arr);
	myfree(buf);
	return 1;
}

/*
	This subroutine is used to myfree continous Blocks of data
*/

int freeBlock(int fd,int index)
{
	struct File1* arr=(struct File1*)mymalloc(sizeof(struct File1)*SIZE);
	if(arr==NULL)
	{
		return -8;
	}
	if(readDiskBlock(fd,index/SIZE,(char*)arr)<0)
	{
		myfree(arr);
		return -4;
	}
	int start=arr[index%SIZE].start;
	int size=arr[index%SIZE].size;
	unsigned char* arr1= (unsigned char*)mymalloc(sizeof(unsigned char)*blksize);
	if(arr1==NULL)
	{
		myfree(arr);
		return -8;
	}
	size=(start+size)-RES;
	start=start-RES;
	int i;
	while(start!=size)
	{
		if(readDiskBlock(fd,(start/(sizeof(unsigned char)*8*blksize))+TSTRUCT,(char*)arr1)<0)
		{
			myfree(arr);
			myfree(arr1);
			return -4;
		}
		for(i=(start/(sizeof(unsigned char)*8*blksize));i==(start/(sizeof(unsigned char)*8*blksize))&start!=size;start++)
		{
			ClearBit(arr1,(start%(sizeof(unsigned char)*8*blksize)));
		}
		if(writeDiskBlock(fd,i+TSTRUCT,(char*)arr1)<0)
		{
			myfree(arr);
			myfree(arr1);
			return -6;
		}
	}
	myfree(arr1);
	arr[index%SIZE].size=0;
	if(writeDiskBlock(fd,index/SIZE,(char*)arr)<0)
	{
		myfree(arr);
		return -6;
	}
	myfree(arr);
	return 1;
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
	struct File1* arr=(struct File1*)mymalloc(sizeof(struct File1)*SIZE);
	if(arr==NULL)
	{
		close(fd);
		return -8;
	}
	if(readDiskBlock(fd,0,(char*)arr)<0)
	{
		close(fd);
		myfree(arr);
		return -4;
	}
	if(arr[0].start==0)
	{
		myfree(arr);
		close(fd);
		return 0;
	}
	int i=arr[0].next;
	int n,p=0,j=0;
	for(n=arr[0].start;n>0;p=i,n--,i=arr[i%SIZE].next)//search for the file to be deleted
	{
		if(readDiskBlock(fd,i/SIZE,(char*)arr)<0)
		{
			close(fd);
			myfree(arr);
			return -4;
		}
		if(strcmp(arr[i%SIZE].filename,filename)==0)
			break;
		j=i;
		while(arr[i%SIZE].last==0)
		{
			i=arr[i%SIZE].next;
			if(readDiskBlock(fd,i/SIZE,(char*)arr)<0)
			{
				close(fd);
				myfree(arr);
				return -4;
			}
		}
	}
	if(n==0)
	{
		close(fd);
		myfree(arr);
		return -19;
	}
	int size=0,index=i;
	while(arr[i%SIZE].last==0)
	{
		size+=arr[i%SIZE].size;
		if(freeBlock(fd,i)<0)
		{
			close(fd);
			myfree(arr);
			return -10;
		}
		i=arr[i%SIZE].next;
		if(readDiskBlock(fd,i/SIZE,(char*)arr)<0)
		{
			close(fd);
			myfree(arr);
			return -4;
		}
	}
	int next=arr[i%SIZE].next;
	size+=arr[i%SIZE].size;
	if(freeBlock(fd,i)<0)
	{
		close(fd);
		myfree(arr);
		return -10;
	}
	if(readDiskBlock(fd,p/SIZE,(char*)arr)<0)
	{		
		close(fd);
		myfree(arr);
		return -4;
	}
	arr[p%SIZE].next=next;
	if(writeDiskBlock(fd,p/SIZE,(char*)arr)<0)
	{
		close(fd);
		myfree(arr);
		return -6;
	}
	if(readDiskBlock(fd,0,(char*)arr)<0)
	{
		close(fd);
		myfree(arr);
		return -4;
	}
	arr[0].start-=1;
	arr[0].size+=size;
	if(arr[0].last==index)
	{
		arr[0].last=p;
	}
	if(writeDiskBlock(fd,0,(char*)arr)<0)
	{
		close(fd);
		myfree(arr);
		return -6;
	}
	close(fd);
	myfree(arr);
	return 1;
}

/*
	This subroutine finds a file of given name into the disk if found it returns its structure index
*/
int search (char* file)
{
	int i=0;
	int fd = open("disk.teasage",O_RDONLY);
	if(fd==-1)
	{
		return -12;
	}
	struct File1* arr =(struct File1*)mymalloc(sizeof(struct File1)*8);
	if(arr==NULL)
	{
		close(fd);
		return -8;
	}
	if(readDiskBlock(fd,0,(char*)arr)<0)
	{
		close(fd);
		myfree(arr);
		return -4;
	}
	if(arr[0].size==0)
	{
		close(fd);
		myfree(arr);
		return 0;
	}
	do
	{
		i=arr[i%SIZE].next;
		if(readDiskBlock(fd,i/SIZE,(char*)arr)<0)
		{
			close(fd);
			myfree(arr);
			return -4;
		}
		if(strcmp(arr[i%SIZE].filename,file)==0)
		{
			close(fd);
			myfree(arr);
			return i;
		}
		if(arr[i%SIZE].last==0)
		{
			while(arr[i%SIZE].last==0)
			{
				i=arr[i%SIZE].next;
				if(readDiskBlock(fd,i/SIZE,(char*)arr)<0)
				{
					close(fd);
					myfree(arr);
					return -4;
				}
			}
		}
	}
	while(arr[i%SIZE].next!=0);
	close(fd);
	myfree(arr);
	return 0;
}

/*
	This subroutine is used to store particular struct at given the index
*/

int getVDFile(int fd,int i,struct File1** res)
{
	struct File1* arr=(struct File1*)mymalloc(sizeof(struct File1)*SIZE);
	if(readDiskBlock(fd,i/SIZE,(char*)arr)<0)
	{
		myfree(arr);
		return -8;
	}
	strcpy((*res)->filename,arr[i%SIZE].filename);
	(*res)->start=arr[i%SIZE].start;
	(*res)->last=arr[i%SIZE].last;
	(*res)->size=arr[i%SIZE].size;
	(*res)->next=arr[i%SIZE].next;
	myfree(arr);
	return 1;
}

/*
	This subroutine is used to comapare two files between actual disk and virtual disk
*/
int compareFiles(char *adfile,char* vdfile)
{
	int fd = open("disk.teasage",O_RDONLY);
	if(fd==-1)
	{
		return -12;
	}
	int fd1=open(adfile,O_RDONLY);
	int i =search(vdfile);
	struct File1* ad=(struct File1*)mymalloc(sizeof(struct File1));
	if(ad==NULL)
	{
		close(fd1);
		close(fd);
		return -8;
	}
	i=getVDFile(fd,i,&ad);
	if(fd1==-1 || i==-1)
	{
		close(fd1);
		close(fd);
		myfree(ad);
		return -13;
	}
	//check bit by bit and also take care of the not continous blocks
	char * buf = (char*)mymalloc(sizeof(char)*blksize);
	char * buf1 = (char*)mymalloc(sizeof(char)*blksize);
	if(buf==NULL || buf1==NULL)
	{
		close(fd1);
		close(fd);
		myfree(ad);
		return -8;
	}
	int rd;
	i = ad->start;
	while((rd=read(fd1,buf,blksize))!=0)
	{
		if(rd==-1)
		{
			close(fd1);
			close(fd);
			myfree(ad);
			myfree(buf);
			myfree(buf1);
			return -5;
		}
		if(readDiskBlock(fd,i,buf1)<0)
		{
			close(fd1);
			close(fd);
			myfree(ad);
			myfree(buf);
			myfree(buf1);
			return -4;
		}
		for(int j=0;j<rd;j++)
		{
			if(buf[j]!=buf1[j])
			{
				close(fd);
				close(fd1);
				myfree(ad);
				myfree(buf);
				myfree(buf1);
				return -10;
			}
		}
		i++;
		if(i==ad->start+ad->size)
		{
			if(ad->last!=0)
			{
				if(ad->last==rd)
				{
					break;
				}
				close(fd);
				close(fd1);
				myfree(buf);
				myfree(buf1);
				myfree(ad);
				return -1;
			}
			else
			{
				if(rd<blksize)
				{
					close(fd);
					close(fd1);
					myfree(buf);
					myfree(buf1);
					myfree(ad);
					return -1;					
				}
				i=ad->next;
				i=getVDFile(fd,i,&ad);
				if(i==-1)
				{
					close(fd);
					close(fd1);
					myfree(ad);
					myfree(buf);
					myfree(buf1);
					return -1;
				}
				i=ad->start;
			}
		}
	}
	if(i!=ad->start+ad->size && ad->last==0)
	{
		close(fd);
		close(fd1);
		myfree(ad);
		myfree(buf);
		myfree(buf1);
		return -1;
	}
	close(fd1);
	close(fd);
	myfree(buf);
	myfree(buf1);
	myfree(ad);
	return 1;
}

/*
	This subroutine is used to check the correctness of the particular action performed on the disk
*/
void test1(FILE* tr,char* str)
{
	int n=strlen(str);
	char *strp[]={NULL,NULL,NULL,NULL};
	strp[0]=strtok(str,"\t");
	int i=0;
	while(strp[i]!=NULL)
	{
		++i;
		if(i==4)
			break;
		strp[i]=strtok(NULL,"\t");
	}
	int fun=myAtoi(strp[0]);
	if(strp[3]!=NULL)
	{
		if(i=myAtoi(strp[3])<=0)// No need to check if there is error in the operation
		{
			return;
		}
	}
	switch(fun)
	{
		case 2://vdcpto
			fprintf(tr,"vdcpto\t%s\t%s\t",strp[1],strp[2]);
			if(compareFiles(strp[1],strp[2])>0)
				fprintf(tr,"1\n");//file are same
			else
				fprintf(tr,"-1\n");//file are not same
			break;
		case 3://vdcpfrom
			fprintf(tr,"vdcpfrom\t%s\t%s\t",strp[1],strp[2]);
			if(compareFiles(strp[1],strp[2])>0)
				fprintf(tr,"1\n");//file are same
			else
				fprintf(tr,"-1\n");//file are not same
			break;
		case 4://delete
			fprintf(tr,"delete\t%s\t",strp[1]);
			if((i=search(strp[1]))<0)
				fprintf(tr,"-1\n");//file not deleted
			else
				fprintf(tr,"1\n");//file deleted
			break;
		case 5://EXIT
				fprintf(tr,"EXIT\n");//EXIT
			break;
		default:
			;
	}
}

/*
	This subroutine is used to check the file and extract the function for checking 
*/

int test()
{
	FILE* lf=fopen("logfile","r");
	FILE* tr=fopen("testResult","w");
	if(lf==NULL || tr==NULL)
		return -13;
	char ch;
	int i=0;
	char*buf = (char*)mymalloc(sizeof(char)*112);
	if(buf==NULL)
	{
		fclose(lf);
		fclose(tr);
		return -8;
	}
	while((ch=getc(lf))!=EOF)
	{
		if(ch=='\n')
		{
			buf[i]='\0';
			test1(tr,buf);
			i=0;	
		}
		else
		{
			buf[i]=ch;
			i++;
		}
	}
	fclose(lf);
	fclose(tr);
	myfree(buf);
	return 1;
}

void errorPrint(FILE* lf,int e)
{
	fprintf(estream,"ERROR:%d\n",e);
	fprintf(lf,"%d\n",e);
}

int main()
{
	int ch,e;
	FILE* lf=fopen("logfile","w");
	if(lf==NULL)
	{
		lf=fopen("logfile","w");
		if(lf==NULL)
			return -13;
	}
	if(createDisk()<0)
	{
		errorPrint(lf,-1);
		fclose(lf);
		return -14;
	}
	if((e=check())<0)
	{
		errorPrint(lf,e);
		fclose(lf);
		return -20;
	}	
	char * path= (char*)mymalloc(sizeof(char)*50);
	char * file = (char *)mymalloc(sizeof(char)*48);
	char *buf= (char*)mymalloc(sizeof(char)*100);
	int * start =(int*)&buf[0];
	*start=0;
	start = (int*)&buf[4];
	*start=100;
	if(buf==NULL || path == NULL || file ==NULL)
	{
		fclose(lf);
		return -8;
	}
	do
	{
		fprintf(ostream,"\n1.List all the files\n2.Copy to VirtuaL Disk\n3.Copy from Virutal Disk\n4.Delete a File\n0.Exit\nEnter your choice\t");
		fscanf(istream,"%d",&ch);
		switch(ch)
		{
			case 1:
				fprintf(lf,"%d\t",ch);		
				while((e=vdls(buf))>0)
				{
					for(int i=0,j=0;i<e;i++)
					{
						fprintf(ostream,"%s\t",buf+8+j);
						fprintf(lf,"%s\t",buf+8+j);
						j+=strlen(buf+8+j)+1;
					}
				}
				if(e<0)
				{
					errorPrint(lf,e);
				}
				else
				{
					fprintf(lf,"1\n");
					start=(int*)&buf[0];
					*start=0;
				}
				break;
			case 2:
				fprintf(ostream,"\nEnter the path\t");
				fscanf(istream," %[^\n]",path);
				fprintf(ostream,"Enter the filename\t");
				fscanf(istream," %[^\n]",file);
				fprintf(lf,"%d\t%s\t%s\t",ch,path,file);
				if((e=vdcpto(path,file))<=0)
				{
					errorPrint(lf,e);
				}
				else
				{
					fprintf(lf,"1\n");
				}
				break;
			case 3:
				fprintf(ostream,"\nEnter the path\t");
				fscanf(istream," %[^\n]",path);
				fprintf(ostream,"Enter the filename\t");
				fscanf(istream," %[^\n]",file);
				fprintf(lf,"%d\t%s\t%s\t",ch,path,file);
				if((e=vdcpfrom(path,file))<=0)
				{
					errorPrint(lf,e);
				}
				else
				{
					fprintf(lf,"1\n");
				}
				break;
			case 4:
				fprintf(ostream,"Enter a filename\t");
				fscanf(istream," %[^\n]",file);
				fprintf(lf,"%d\t%s\t",ch,file);
				if((e=deleteFileFromDisk(file))<=0)
					errorPrint(lf,e);
				else
					fprintf(lf,"1\n");
				break;
			case 0:
				fprintf(lf,"5\n");
				break;
			default:
				fprintf(lf,"INVALID CHOICE\n");
		}
	}while(ch!=0);
	myfree(path);
	myfree(buf);
	myfree(file);
	fclose(lf);
	if(test()<0)
		fprintf(ostream,"TESTING ERROR\n");
	return 1;
}
