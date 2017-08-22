/* 
 * File:   queue.h
 * Author: nslab1
 *
 * Created on 24 November, 2014, 2:28 PM
 */

#ifndef QUEUE_H
#define	QUEUE_H
#include<iostream>
#include<string.h> 
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

struct Qnode
{
	int data;
	struct Qnode *next;
};

class queue
{
	public:
		int numberItems;
		struct Qnode *rear;
		struct Qnode *front;

		queue() 			// constructor
		{
			front=NULL;
			rear=NULL;
			numberItems = 0;
		}
		~queue(){
			front = rear = NULL;
			numberItems = 0;
		}	// destructor


		void enqueue(int); 			// to insert an element in front
		void display(); 			// to display all elements
		void dequeue(int);  			// to delete an element from rear
		int count();
		struct Qnode* getFirst();
		int getValue(struct Qnode* r);
};

#endif	/* QUEUE_H */

