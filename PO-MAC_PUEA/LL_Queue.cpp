#include"queue.h"

using namespace std;

/*

 * Insert Element into the Queue

 */
void queue::enqueue(int item)
{

	struct	Qnode *tmp;

	tmp = new (struct Qnode);
	tmp->data = item;

	tmp->next = NULL;

	if (rear == NULL)
		rear =	front = tmp;
	else{
		rear->next = tmp;
		rear = tmp;
	}

	numberItems++;
}
/*

 * Delete Element from the Queue

 */

void queue::dequeue(int value)
{
	struct Qnode *tmp;
	struct Qnode *prev;

	tmp = front;

	if (front == NULL)
	{
		cout<<"Queue Underflow"<<endl;
		return;
	}
	while(  (tmp != NULL) && (tmp->data != value))
	{
		prev = tmp;
		tmp = tmp->next;
	}

	if(tmp == NULL)
	{
		printf("element not found\n");
	}
	else if(tmp == front){

		front = front->next;
		tmp->next = NULL;
		numberItems--;
		//cout<<"Element Deleted: "<<tmp->data<<endl;
		delete tmp;
	}else if(tmp == rear)
	{
		rear = prev;
		tmp->next = NULL;
		//cout<<"Element Deleted: "<<tmp->data<<endl;
		numberItems--;
		delete tmp;
	}
	else{
		prev->next = tmp->next;
		tmp->next = NULL;
		numberItems--;
		//cout<<"Element Deleted: "<<tmp->data<<endl;
		delete tmp;
	}

}

/*

 * Traversing the Stack

 */

void queue::display()
{       

	struct	Qnode *ptr;

	ptr = front;

	if (front == NULL)
		cout<<"Queue is empty"<<endl;
	else
	{

		//cout<<"Queue elements :"<<endl;

		while (ptr != NULL)
		{
			//cout<<ptr->data<<" ";
			ptr = ptr->next;
		}
		//cout<<endl;
	}
}


struct Qnode* queue::getFirst() {
	return front;
}

int queue::getValue(struct Qnode* r) {

	if(r!= NULL)
		return r->data;
}

int queue:: count(){
	return numberItems;
}

