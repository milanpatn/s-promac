#include "channel.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>



using namespace std;

channel::channel() {
	chId = 0;	
	chFreq = 0.0;
	busyFlag = 0;
	pthread_mutex_init(&chMutex,NULL);
	busyPeriod = 0.0;
	freePeriod = 0.0;
	pTxPwr = 0.0;
	start=NULL;
	end=NULL;
	count=0;	
	patterncount=0;	
	userid = 0;
	pudist = 500;
}

channel::channel(int n,double f) {
	chFreq = f;
	busyFlag = 0;
	pthread_mutex_init(&chMutex,NULL);
	chId = n ;
	busyPeriod = 0.0;
	freePeriod = 0.0;
	pTxPwr = 0.0;
	start=NULL;
	end=NULL;
	count=0;	
	patterncount=0;	
	userid = 0;
	pudist = 500;
}

channel::~channel() {
	pthread_mutex_destroy(&chMutex);
}

void channel:: setChId(int i) {
	chId = i;
}

void channel:: setChFreq(double f) {
	chFreq = f;
}

void channel::setBusyFlag(int b){
	busyFlag = b;
}

void channel::setChReq(int r) {
        chReq = r;
}

void channel::setBusyPeriod(double p) {
	busyPeriod = p;
}

void channel::setFreePeriod(double p) {
	freePeriod = p;
}

double channel::getFreePeriod() {
	return	freePeriod;
}

int channel::getChId() {
	return chId;
}

double channel::getChFreq(){
	return chFreq;
}

int channel::getBusyFlag(){
	return busyFlag;
}

int channel::getChReq() {
	return chReq;
}

double channel::getBusyPeriod() {
	return busyPeriod;
}

void channel::setPTxPwr(float pwr){//----------------------
	pTxPwr = pwr;
}

float channel::getPTxPwr(){//----------------------
	return pTxPwr;
}

void channel::setOptDist(float dist){//-----------
	optDist = dist;
}

float channel::getOptDist(){//---------------
	return optDist;
}

int channel::lockChannel(){

	if( pthread_mutex_trylock(&chMutex) == 0)
//if ebusy try again
	{
//		pthread_mutex_lock(&chMutex);
		busyFlag=1;
		return 1;
	}
	else
		return 0;
}

void channel::unlockChannel(){
	busyFlag = 0;	
	pthread_mutex_unlock(&chMutex);
}

/* TO create an empty dlnode */
struct dlnode* channel::create(int data)
{
	struct dlnode*   temp = NULL;
	temp =(struct dlnode *)malloc(1*sizeof(struct dlnode));

 memset(temp, '\0', sizeof(struct dlnode));

	temp->prev = NULL;
	temp->next = NULL;
	temp->data = data;
	return temp;
}

/* To delete an element */
void channel::removee()
{
	dlnode *temp=NULL;

	if (start == NULL)
	{
		printf("\n Error : Empty list no elements to delete");
		return;
	}
	else
	{
		temp=start;
		start= start->next;
		temp->next=NULL;
		start->prev=NULL;
		free(temp);
	}
}

/*  TO insert at beginning */
void channel::insert1(int data)
{

	struct dlnode* temp;
	if (start == NULL)
	{
		count=0;
		start =  create(data);
		start->prev=NULL;
		start->next=NULL;
		end=start; 
	}
	else
	{
		temp= create(data);
		end->next = temp;
		temp->next = NULL;
		temp->data = data;
		temp->prev=end;
		end = temp;
	}

	count++;

	if (count > HIST_SIZEE)
	{
//		printf("remove called for %d\n",getChId());
		removee();
	}
}

void channel::traverse()
{
	struct dlnode* temp =start;

	while (temp != NULL)
	{
		printf(" %d ", temp->data);
		temp = temp->next;
	}
}

void channel::traverse_pattern(int count)
{
	int i;
	for(i=0;i<count;i++)
	{
		printf("%d ",pattern[i]);

	}

}

struct dlnode* channel::get_history()
{
	return end; 
}	

void channel::set_history(int data)
{
	insert1(data);
}


int* channel::get_pred()
{
	return future; 
}

void channel::set_pred(int data[10]){

	for(int i=0;i<PRED_RANGE;i++)
	{
		future[i]=data[i];

	}

}
