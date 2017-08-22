#include "node.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#define DEBUG 0

using namespace std;
extern channel* chArray; // list of PU channels
extern pthread_mutex_t QMutex;
int flag=1;
extern int exitflag;
extern double matlab_time;
extern double scanTime;
secondary secondUserArr[NUM_SEC_USERS];// list of SU users
primary primaryUserArr[NUM_PRIMARY_USERS];// list of PU users
extern int counter[(NUM_PRIMARY_USERS + NUM_SEC_USERS)]; //Used to bring all task to suspension
extern struct timeval sysstart;
int turn;
extern int max_secusers;
/*
Reactive algorithm to find the next state based on the histiry
of previous states. 
This algorithm can be tweaked in the below function.
*/
bool decision(int *data)
{
	if( (data[0] == 1 || data[1] == 1 || data[2] == 1))
		return true;
	else
		return false;
}
/*
Stores the past three prediction decisions
*/

bool predict(int s,struct dlnode* start,int pattern[26]){

	int k=0,i=0,j=0;
	struct dlnode* temp=NULL;
	int flag1=0;
	int data[PRED_RANGE];

	for(k=0;(k+HIST_SIZEE)<=s;k++)
	{
		i=k;
		for ( temp=start;  temp != NULL && (i < s) ; )
		{
			if (pattern[i] == temp->data ) {

				i++;
				j++;
				temp = temp->next;
			}
			else
			{ 
				i= i+1;
				j=0;
				temp =start;
			}
		}

		if(j==HIST_SIZEE)
		{
			//printf("\nmatch from i=%d\n",i);
			flag1=1;

			if(i+3<= s)
			{
			//	printf("next values are %d %d %d\n",pattern[i],pattern[i+1],pattern[i+2]);
				data[0]=pattern[i];data[1]=pattern[i+1];data[2]=pattern[i+2];
			}
			else if(i+2==s)
			{	
			//	printf("next values are %d %d %d\n",pattern[i],pattern[i+1],pattern[0]);
				data[0]=pattern[i];data[1]=pattern[i+1];data[2]=pattern[0];
			}
			else if(i+1==s)
			{
			//	printf("next values are %d %d %d\n",pattern[i],pattern[0],pattern[1]);
				data[0]=pattern[i];data[1]=pattern[0];data[2]=pattern[1];
			}
			else if(i==s)//crossed the last element so go to first element
			{
			//	printf("next values are %d %d %d\n",pattern[0],pattern[1],pattern[2]);
				data[0]=pattern[0];data[1]=pattern[1];data[2]=pattern[2];
			}
			break;
		}

		if(flag1 == 1)
		{
			return( decision(data));//take decision from predicted data
		}
	}
	return false;
}

/*
   Brief - A timer value generator. 
   Used for exponential Backoff
 */
double Exponential(double mean)
{
	double R,x;
	R = (double)rand()/(double)(RAND_MAX);
	x =  -mean*log(R);

	if( x > ( (double) 5) )
		return (double)5;
	else
		return x;

}

void node::setRxdist() {//--------------------------------------------------
	//Rxdist = 0.85+(rand() % 1);
	Rxdist = Exponential(0.3);
}


void node::setTxpwr(float p) {//---------------------------------
	Txpwr = p;
}

void node::setOpttxpwr(float pwr){//-------------------------
	Opttxpwr = pwr;
}

void node::setId(int id) {
	nodeId = id;
}

void node::setBusyFlag(int f) {
	busyFlag = f;
}

void node::setChannel(channel* c) {
	ch = c;
}

void node::setTxtime(double s) {
	Txtime = s;
}

int node::getId() {
	return nodeId;
}

int node::getBusyFlag() {
	return busyFlag;
}

channel* node::getChannel() {
	return ch;
}

double node::getTxtime() {
	return Txtime;
}

/*
Constructor: Initializes the object to default values
 */
primary::primary() {
	nodeId = 0;
	busyFlag = 0;
	ch = NULL;
	Txpwr = 0.0;//------------
	Rxdist = 0.0;//------------
	Opttxpwr = 0.0;//-----------------	
	Txtime=0.0;
	runtime = 0.0;
	freetime = 0.0;
	busytime = 0.0;
	trflag[1] = false;
	trflag[0] = false;
}

primary::primary(int id, channel*c) {
	nodeId = id;
	busyFlag = 0;
	ch = c;
	Txpwr = 0.0;//------------
	Rxdist = 0.0;//------------
	Opttxpwr = 0.0;//-----------------
	Txtime=0.0;
	runtime = 0.0;
	freetime = 0.0;
	busytime = 0.0;
	trflag[1] = false;
	trflag[0] = false;
}

primary::~primary() {
}

/*
   To be used in Primary user alone.
   Redefining transmission time in terms of time slots
   exact calculated value of the markov eqn.
   Use the smallest of fi,ft as the size of unit time slot
   By this we get n time slots for each distribution.

   Transmission time remains almost same for all transmissions,
   but idle time increases from left to right. 


   Calculated transmission time(ft)

   0.593796       0.603836        0.614179        0.621248        0.624836        0.621248        0.621248

   Calculated idle time(fi)

   0.060956       0.104067        0.200057        0.291658        0.396525        0.512968        0.991451

   Reference - S. Geirhofer, L. Tong, B. M. Sadler, Cognitive medium access: constraining interference based on
   experimental models, Selected Areas in Communications, IEEE Journal on 26 (1) (2008) 95–105.

 */

double ft( double mu) 
{
	double result =0.0;

	result = 1 - exp( -(1/mu) );

	return result*5;
}


double fi(double lambda)
{
	double result =0.0;

	result = 1 - exp( -(1/lambda) );

	return result*5;
}

/*
   Brief - A PU task. Used by every PU task and executes the below functionality.
   Each PU tassk can execute its functionality under three different load conditions.
 */

void* primary::thread_maker(void* data)
{
	primary *p = static_cast<primary*>(data);

	double Ft=3,Fi=1;
	bool time_elapsed = true;
	unsigned int fti=0;

	/*Markov continuous distribution*/
//	double table1[7] = {15.9, 9.1,4.48,2.9,1.98,1.39,.21};//lambda-fi
//	double table2[7] = {1.11,1.08,1.05,1.03,1.02,1.03,1.03};//mu-ft

// busy traffic
	double table1[7] = {15.9, 9.1,15.9, 9.1,4.48, 9.1,4.48};//lambda-fi
	double table2[7] = {1.11,1.08,1.11,1.08,1.05,1.08,1.05};//mu-ft

	//mostly less traffic
//	double table1[7] = {1.39,1.98,1.39,1.98 ,1.39,1.98,1.98};//lambda-fi
//	double table2[7] = {1.03,1.02,1.03,1.02,1.03,1.02,1.02};//mu-ft

	int i=0,j=0;
	char filename[80];
	char ta[80];
	FILE * fp;
	int rid = 1;

	//	sprintf(filename,"%d.txt",p->getId());
	//	fp = fopen (filename, "w");

	struct timeval start;
	struct timeval end;
	srand(time(NULL));
	fti= p->getId()%6;
	Fi = fi(table1[fti]); // Idle time for the channel
	Ft = fi(table2[fti]); // Transmission time for the channel

	/*meta data*/
	//	sprintf(ta,"%d\n",fti);
	//	fputs(ta,fp);

	//	fputs("pattern - ",fp);

		for(i=0;i<Ft/.30478;i++)
		{
		//fputs("0 ",fp);
		p->getChannel()->pattern[i]=0;
		}
		int k=(Ft/.30478);
		for(i=0;i<Fi/.30478;i++)
		{
		p->getChannel()->pattern[i+k]=1;
	//	sprintf(ta,"%d ",p->getChannel()->pattern[i+k]);
	//	fputs(ta,fp);
		}
		p->getChannel()->patterncount = Ft/.30478 + Fi/.30478;
	//	fclose(fp);
	/*
	   Beginning of the PU task executions
	 */
	while(1  && (exitflag == 0)) {
		//calculate Tx time for each PU 
		p->setTxtime(Ft);

		/*
		   This make sure that not many users transmit on a PU channel. 
		   Also the second check is to exit immediately, incase simulation time is over
		 */

		while(p->trflag[1] == true && (exitflag == 0)){
			//			printf("\n  Transmitter is %d blocked as receiver flag set\n", s->getId() );
		}

		rid = (unsigned)rand()%NUM_PRIMARY_USERS;	

		//if the receiver is busy, wait
		while( (primaryUserArr[rid].trflag[0] == true || primaryUserArr[rid].trflag[1] == true) && (exitflag == 0));

		p->trflag[0] = true;
		p->trflag[1] = false;

		primaryUserArr[rid].trflag[1] = true;
		primaryUserArr[rid].trflag[0] = false;


		gettimeofday(&start, NULL);

		while(!p->getChannel()->lockChannel()) { //trying to lock the allocated channel

			p->getChannel()->setChReq(1);//req for channel 

			//			printf(" channel no: %d  request made by PU %d held by SU %d\n", p->getChannel()->getChId(), p->getId() , p->getChannel()->userid);

			if(exitflag == 1)
				break;

			usleep(BACKOFF_TIME);//waiting

		}

		/*Busy Time*/
		if( i> 3  ) // every 2 cycles randomly select one patter
		{
			srand(time(NULL));
			fti= (unsigned)rand()%7;
			Fi = fi(table1[fti]);
			Ft = fi(table2[fti]);
			//			fp = fopen (filename, "a+");
			//			sprintf(ta,"\n%d\n",fti);
			//			fputs(ta,fp);
			//			fputs("pattern - ",fp);
			
						for(i=0;i<Ft/.30478;i++)
						{
						//fputs("0 ",fp);
						p->getChannel()->pattern[i]=0;
						}
						k=(Ft/.30478);
						for(i=0;i<Fi/.30478;i++)
						{
						p->getChannel()->pattern[i+k]=1;
			//			sprintf(ta,"%d ",p->getChannel()->pattern[i+k]);
					//	fputs(ta,fp);
						}
						p->getChannel()->patterncount = Ft/.30478 + Fi/.30478;
					//	fclose(fp);
			 			i=0;
		}
		i++;


		p->setBusyFlag(1);// set busy flag


		p->txData( );// send data
		
		p->trflag[0] = false;
		p->trflag[1] = false;
		primaryUserArr[rid].trflag[1] = false;
		primaryUserArr[rid].trflag[0] = false;

		p->setBusyFlag(0);
		p->getChannel()->unlockChannel();// release channel

		gettimeofday(&end, NULL);

		double runTime = (end.tv_sec + (end.tv_usec/1000000.0)) - (start.tv_sec + (start.tv_usec/1000000.0));	
		//		printf("RUMTIME OF PRIMARY %d = %f fti %d\n", p->getId(),runTime,fti); 

		unsigned  s = Fi/0.30478; //0.060956*5 since ft & fi return result multiplied by 5

		/*Idle times*/
		for(j=0;j<s ;j++)
		{
			//channel currently not under transmission, so available
			p->getChannel()->set_history(1);

			if(exitflag == 1)
				break;

			usleep( 0.30478*1e6 ); 
			p->freetime += (double)0.30478;
			//p->getChannel()->setFreePeriod(p->getChannel()->getFreePeriod() + (double)0.30478);

		}
	}
	pthread_exit(NULL);
}


/*
   Brief- The function does transmission of PU data.
   The data is transmitted for time "getTxtime()"

   Parameters affected - set_history, busy_time & setBusyPeriod

 */
void primary::txData( ) 
{
	usleep(SENSING_TIME);
	printf("PU %d ACQUIRED CHANNEL %d AND DATA TXN STARTS !!\n",this->nodeId,this->ch->getChId());
	unsigned  s = this->getTxtime()/0.30478;

	//FOR ATTACK STARTS
	int attack = 0;
	attack = this->nodeId;

	
	if(attack < 102){
		printf("*******The %d Attacker transmission !!********\n", attack);
		//Fi = 0;
		//Ft = 10000;
		s = 100000;
	}

	//FOR ATTACK ENDS



	for(int j=0;j<s  ;j++)
	{
		this->getChannel()->set_history(0);
		//channel currently under transmission, so not available
		this->getChannel()->userid = this->getId();

		if(exitflag == 0){
			usleep( 0.30478*1e6 ); 
		}		else
		{
			break;
		}

		this->busytime += (double) 0.30478;
		this->getChannel()->setBusyPeriod(this->getChannel()->getBusyPeriod() + (double) 0.30478); //busy time range
		//		printf("\n user %d  busy on channel: %d - %f's \n", this->getId(),this->ch->getChId(),this->getChannel()->getBusyPeriod());
	}
		this->getChannel()->userid = 99;

	// once the channel is fully used by Pu. set to '0'. So that others can use.
	this->getChannel()->setChReq(0);
	printf("PU %d RELEASED CHANNEL %d AND DATA TXN STOPS !!\n",this->nodeId,this->ch->getChId());		
}

/*
Constructor: Initializes the object to default values
 */
secondary::secondary() {
	nodeId = 0;
	numOfBackoffs = 0;
	numOfFailures = 0;//---------
	numOfInterference = 0;//------------	
	channelList = NULL; 
	Txtime=0.0;
	compTx=0;
	runtime = 0.0;
	freetime = 0.0;
	busytime = 0.0;
	failure = 0;	
trflag[1] = false;
	trflag[0] = false;
}

secondary::secondary(int id, queue* c) {
	nodeId = id;
	channelList = c;
	numOfBackoffs = 0;
	numOfFailures = 0;//----------------
	numOfInterference = 0;//-------------------
	Txtime=0.0;
	compTx=0;
	runtime = 0.0;
	freetime = 0.0;
	busytime = 0.0;
	failure = 0;	
trflag[1] = false;
	trflag[0] = false;

}

secondary::~secondary() {
}

void secondary::setChannelList(queue* q)
{
	channelList = q;
}

queue* secondary::getChannelList()
{
	return channelList;
}

void secondary::incNumOfBackoffs() {
	numOfBackoffs++;
}

int secondary::getNumOfBackoffs() {
	return numOfBackoffs;
}
//----------------------------------------------------
void secondary::incNumOfFailures() {
	numOfFailures++;
}

int secondary::getNumOfFailures() {
	return numOfFailures;
}

void secondary::incNumOfInterference() {
	numOfInterference++;
}

int secondary::getNumOfInterference() {
	return numOfInterference;
}

int secondary::getcompletedTx() {
	return compTx;
}

void secondary::setcompletedTx() {
	++compTx;
}
//---------------------------------------------------


/*
   Brief - It is a SU task. A task is created for each SU. And each SU executes the functionality in the below routine.

*/
void* secondary::thread_maker(void* data) {

	secondary *s = static_cast<secondary*>(data);

	int count, nextIndex;
	struct timeval mstart;
	struct timeval mend;
	int i=0;
	int a=0;
	FILE *fp = NULL;
	FILE *fp1= NULL;
	int read[80];
	double sumtime = 0.0;
	struct timeval start,end;
	struct timeval sumstart,sumend;
	int rid = 1;
	double runTime = 0;
	turn = 2;	
	while(1 && (exitflag == 0)) {

		struct timeval currentTime;
		gettimeofday(&currentTime, NULL);
		/**** Only for Sec User whose turn it is, to run channel prediction algorithm****/
		/**** start ****/
		if( s->getId() == turn &&  (exitflag == 0) )//every 5th sec run mpc in SU 1
		{
			//start prediction

			nextIndex = 0;

			double rt = .5*(rand()%NUM_SEC_USERS);
			flag=0;
			sleep(rt);
			pthread_mutex_lock(&QMutex);
			s->runtime += rt;
			gettimeofday(&start, NULL);

			count = 0;
			struct Qnode* temp = s->channelList->getFirst();	

			do {
				nextIndex = s->channelList->getValue(temp);


				bool decision = predict( (chArray + nextIndex)->patterncount, (chArray + nextIndex)->start, (chArray + nextIndex)->pattern);
				//				bool decision = true;

				if( decision == true )
				{

				}else{
					s->channelList->removeNode(nextIndex);
					s->channelList->enqueue(nextIndex);
				}

				temp = temp->next;
				count++;

			}while(count < s->channelList->count()&& temp!=NULL );

			gettimeofday(&end, NULL);
			runTime = (end.tv_sec + (end.tv_usec/1000000.0)) - (start.tv_sec + (start.tv_usec/1000000.0));
			scanTime += runTime;
			pthread_mutex_unlock(&QMutex);
			flag = 1;

			//			printf("Scan Time = %f\n", scanTime );

		}

		/**** end ****/

		/****** trans-recieve algo******/
		int k=0;

		rid = (unsigned)rand()%NUM_SEC_USERS;	
		s->setTxtime(Exponential(5));
		s->setRxdist();//---------------------


		while(s->getTxtime() >= 0.0 && (exitflag == 0)) {


			gettimeofday(&start, NULL);


			int nextIndex;
			struct Qnode* temp;


			if(flag == 1)
				temp = s->channelList->getFirst();

			for(int i=0;i <  s->channelList->count() && temp!=NULL && flag==1;i++) 
			{

				usleep(BACKOFF_TIME);

				while(s->trflag[1] == true && (exitflag == 0));

				while( (secondUserArr[rid].trflag[0] == true || secondUserArr[rid].trflag[1] == true) && (exitflag == 0));

				nextIndex = s->channelList->getValue(temp);
				temp = temp->next;

				if( nextIndex > 0 && nextIndex < NUM_CHANNELS )	
				{	
					if( (chArray + nextIndex)->getChReq() == 0 )
					{
						if( ((chArray + nextIndex)->lockChannel()) == 1  )
						{
							s->setBusyFlag(1); 

							s->trflag[0] = true;
							s->trflag[1] = false;
							secondUserArr[rid].trflag[1] = true;
							secondUserArr[rid].trflag[0] = false;

							s->setChannel(&chArray[nextIndex]); 

							s->txData();

							s->setBusyFlag(0);

							s->trflag[0] = false;
							s->trflag[1] = false;
							secondUserArr[rid].trflag[1] = false;
							secondUserArr[rid].trflag[0] = false;

							s->getChannel()->unlockChannel();
							break;				
						}
						else{
							s->failure++;				


						}					

					}
				}
			}
			gettimeofday(&end, NULL);
			runTime = (end.tv_sec + (end.tv_usec/1000000.0)) - (start.tv_sec + (start.tv_usec/1000000.0));	
			s->runtime += runTime;
		}

		if( s->getId() == turn &&  (exitflag == 0) )
		{
			//turn = rand()%NUM_SEC_USERS;
		}

		double ss = Exponential(5);
		sleep(ss);
		s->freetime += ss;
	}
	pthread_exit(NULL);
}	
/*
   Brief- The function does transmission of SU data.
   The data is transmitted for time "getTxtime()"

   Parameters affected - incNumOfInterference, incNumOfBackoffs, busytime
*/

void secondary::txData( ) {
	usleep(SENSING_TIME);
	printf("SU %d ACQUIRED CHANNEL %d AND DATA TXN STARTS !!\n",this->nodeId,this->ch->getChId());

	float cutoffdist;//-----------
	cutoffdist = pow(10,(((this->getChannel()->getPTxPwr())+2+90)/NUM_SEC_USERS))/(41.88*(this->getChannel()->chFreq));//------------


	while(this->getTxtime() >= 0.0 ) {

		if(this->ch->getChReq()==1 ) { 
			this->incNumOfBackoffs();
			printf("\n-----------------------------SU %d BACKING OFF FROM CHANNEL %d\n",this->nodeId,this->ch->getChId());
			return;
		}

		if(this->Rxdist > cutoffdist ) {
			this->incNumOfInterference();
		}

		if(exitflag == 0)
		{
			usleep(14.2e4);
		}else
		{
			break;
		}

		this->getChannel()->setBusyPeriod(this->getChannel()->getBusyPeriod() + (double )0.142 );
		this->busytime += (double)  0.142;

		//		printf("\n user %d  busy on channel: %d - %f's \n", this->getId(),this->ch->getChId(),this->getChannel()->getBusyPeriod());

		this->Txtime = this->Txtime - (0.142);

	}

	this->getChannel()->userid = 99;

	this->setcompletedTx() ;
	printf("SU %d RELEASES CHANNEL %d AND DATA TXN STOPS !!\n",this->nodeId,this->ch->getChId());
}

