#include<stdio.h>
#include<stddef.h>
#include"myalloc.h"
#define MAX 10000
typedef max_align_t Align;
union header//basic unit which will be used for the memory allocation
{
	struct 
	{
		union header* next;
		unsigned size;
	}s;
	Align x;//forced allignement to maximun allignement
};
typedef union header Header;

static Header free1[MAX];//free memory buffer
static Header *freep=NULL;//free meory pointer
static Header base;// a empty list 
void myfree(void *ia)
{
	Header *node,*currp,*prevp;
	node=(Header*)ia-1; //points to block header where we have the information of the block stored
	if(node->s.next!=NULL)
	{
		printf("This memeory not allocated by mymalloc\n");
		return;
	}
	for(currp=freep;!(node > currp && node < currp->s.next) ;prevp=currp,currp=currp->s.next)
	{
		if(currp >=currp->s.next && (node > currp || node< currp->s.next))// the node memory is either at start or end existing memory block
		{
			break;
		}
	}
	if( node+ node->s.size ==currp->s.next)//if the memory is continous
	{
		node->s.size+=currp->s.next->s.size;
		node->s.next=currp->s.next->s.next;
	}
	else
		node->s.next=currp->s.next;
	if(currp+currp->s.size==node)
	{
		currp->s.size+=node->s.size;
		currp->s.next=node->s.next;
	}
	else
		currp->s.next=node;
	freep=currp;
}

void * mymalloc(unsigned memreq)
{
	Header *currp,*prevp;
	unsigned nunits;
	nunits =(memreq + sizeof(Header)-1)/sizeof(Header) +1;
	if((prevp=freep)==NULL)
	{
		free1[0].s.next=freep=prevp=&base;
		base.s.size=0;
		base.s.next=&free1[0];
		free1[0].s.size=MAX;
	}
	for(currp=prevp->s.next;;prevp=currp,currp=currp->s.next)
	{
		if(currp->s.size >=nunits)
		{
			if(currp->s.size==nunits)
			{
				prevp->s.next=currp->s.next;
			}
			else
			{
				currp->s.size-=nunits;
				currp+=currp->s.size;
				currp->s.size=nunits;
			}
			currp->s.next=NULL;
			freep=prevp;
			return(void*)(currp+1);
		}
		if(currp==freep) //no adqeuate memory in list
			return NULL;// no memory left;
	}
}

