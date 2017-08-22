#include<iostream>
#include<string> 
#include<stdio.h>
#include<pthread.h>
#include"queue.h"
 
using namespace std;

/* IMPLEMENTATION OF QUEUE USING LINK LIST */

/*   To create a Qnode of link list to store the data */


 
/* To create a class queue with link list */


/* enqueue*/

int queue::frontOfQ()
{
return front -> data;
}

int queue::rearOfQ()
{
	return rear -> data;
}

void queue::enqueue(int s)
{
   struct Qnode *temp;
   temp=new Qnode;
   temp->data=s;
   temp->next=NULL;
   if(rear!=NULL){
      rear->next=temp;
      rear=temp;
   }
   else {front=rear=temp;}
	numberItems++;

//printf("In enq ...Rear of Q is %d -- front of Q is %d\n", rear->data, front -> data);
}
 
/* dequeue */

int queue::dequeue()
{
	struct Qnode *temp = NULL;
	if(front != NULL)
	{
		temp =  front;
		front = front -> next;
		int n = temp -> data;
		temp->next = NULL;
	 delete temp;
	numberItems--;
	//printf("deq....Rear of Q is %d -- front of Q is %d\n", rear->data, front -> data);
	return n;
}
else
{
printf("Nothing to deq&&&&&&&&&&&& Rear of Q is %d -- front of Q is %d\n", rear->data, front -> data);
	return -1;
}
   
}

struct Qnode* queue::getFirst() {
	return front;
}

int queue::getValue(struct Qnode* r) {
	if(r!= NULL)
		return r->data;

}

void queue::removeNode(int value) {
	struct Qnode* prev;
	struct Qnode* cur;
	prev = NULL;
	cur = front;

	while( cur->data != value) {
		prev = cur;
		cur = cur -> next;
	}

	if(cur == rear ) {
		if(prev != NULL)
		{
			prev->next = NULL;
		}
		rear = prev;
		delete cur;		
	}
	else	if(cur == front ){
		front=front->next;	
		delete cur;
	}
	else{
		prev->next = cur->next;
		delete cur;
	}
	numberItems--;	
}

int queue:: count(){
	return numberItems;
}





