/* 
 * File:   queue.h
 * Author: nslab1
 *
 * Created on 24 November, 2014, 2:28 PM
 */

#ifndef QUEUE_H
#define	QUEUE_H
#include<iostream>
#include<string> 
#include<stdio.h>
#include<pthread.h>
struct Qnode
{
	int data;
	struct Qnode *next;
};
class queue
{
	private : 
		struct Qnode *front;		//front of list is rear of queue
		struct Qnode *rear;		//rear of list is the front of queue
		int numberItems;
	
	public:
		queue() 			// constructor
		{
		front=NULL;
		rear=NULL;
		}
      		~queue(){front = rear = NULL;}	// destructor
      		void enqueue(int); 			// to insert an element in front
      		int dequeue();  			// to delete an element from rear
		int frontOfQ();
		int count();
		int rearOfQ();
		struct Qnode* getFirst();
		void removeNode(int value);
		int getValue(struct Qnode* r);
};

#endif	/* QUEUE_H */

