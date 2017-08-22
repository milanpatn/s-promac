#include "node.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#define DEBUG 0

using namespace std;
extern channel* chArray; // list of PU channels
extern pthread_mutex_t QMutex;
extern pthread_mutex_t FMutex;
int flag=1;
extern int exitflag;
extern double scanTime;
secondary secondUserArr[NUM_SEC_USERS];// list of SU users
primary primaryUserArr[NUM_PRIMARY_USERS];// list of PU users
extern int counter[(NUM_PRIMARY_USERS + NUM_SEC_USERS)]; 
extern struct timeval sysstart;
int turn;
extern int max_secusers;
int free_channel[NUM_CHANNELS];//list of free channels
int suregistry[NUM_SEC_USERS];
int su_chan[NUM_SEC_USERS];
int Ns;

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
	ch = NULL;
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
	unsigned int fti=0;

	/*Markov continuous distribution*/
	double table1[7] = {15.9, 9.1,4.48,2.9,1.98,1.39,.21};//lambda-fi
	double table2[7] = {1.11,1.08,1.05,1.03,1.02,1.03,1.03};//mu-ft

	// PRIMARY USER EMULATION ATTACK 1 NODE
	//double table1[7] = {0, 9.1,4.48,2.9,1.98,1.39,.21};//lambda-fi
	//double table2[7] = {100,1.08,1.05,1.03,1.02,1.03,1.03};//mu-ft

	// busy traffic
	//double table1[7] = {15.9, 9.1,15.9, 9.1,4.48, 9.1,4.48};//lambda-fi
	//double table2[7] = {1.11,1.08,1.11,1.08,1.05,1.08,1.05};//mu-ft

	//mostly less traffic
	//double table1[7] = {1.39,1.98,1.39,1.98 ,1.39,1.98,1.98};//lambda-fi
	//double table2[7] = {1.03,1.02,1.03,1.02,1.03,1.02,1.02};//mu-ft

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
	/*	sprintf(ta,"%d\n",fti);
		fputs(ta,fp);

		fputs("pattern - ",fp);

		for(i=0;i<Ft/.30478;i++)
		{
	//fputs("0 ",fp);
	p->getChannel()->pattern[i]=0;
	}
	int k=(Ft/.30478);
	for(i=0;i<Fi/.30478;i++)
	{
	p->getChannel()->pattern[i+k]=1;
	sprintf(ta,"%d ",p->getChannel()->pattern[i+k]);
	//	fputs(ta,fp);
	}
	p->getChannel()->patterncount = Ft/.30478 + Fi/.30478;
	//	fclose(fp);
	 */
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


		while(!p->getChannel()->lockChannel()) { //trying to lock the allocated channel

			p->getChannel()->setChReq(1);//req for channel 

			//			printf(" channel no: %d  request made by PU %d held by SU %d\n", p->getChannel()->getChId(), p->getId() , p->getChannel()->userid);

			if(exitflag == 1)
				break;

			usleep(BACKOFF_TIME);//waiting

		}

		/*Busy Time*/
		if( i > 3  ) // every 2 cycles randomly select one patter
		{
			srand(time(NULL));
			fti= (unsigned)rand()%7;
			Fi = fi(table1[fti]);
			Ft = fi(table2[fti]);
			/*			fp = fopen (filename, "a+");
						sprintf(ta,"\n%d\n",fti);
						fputs(ta,fp);
						fputs("pattern - ",fp);

						for(i=0;i<Ft/.30478;i++)
						{
			//fputs("0 ",fp);
			p->getChannel()->pattern[i]=0;
			}
			k=(Ft/.30478);
			for(i=0;i<Fi/.30478;i++)
			{
			p->getChannel()->pattern[i+k]=1;
			sprintf(ta,"%d ",p->getChannel()->pattern[i+k]);
			//	fputs(ta,fp);
			}
			p->getChannel()->patterncount = Ft/.30478 + Fi/.30478;
			//	fclose(fp);
			 */	 			i=0;
		}
		i++;

		gettimeofday(&start, NULL);

		p->trflag[0] = true;
		p->trflag[1] = false;

		primaryUserArr[rid].trflag[1] = true;
		primaryUserArr[rid].trflag[0] = false;

		p->setBusyFlag(1);// set busy flag

		/*
		int attack = p->getId();

	
		if(attack < 102){
			printf("The %d Attacker transmission !!!!!!!!!!!!!!\n", attack);
			Fi = 0;
			Ft = 10000;
		}
		*/
		p->txData( );// send data

		p->trflag[0] = false;
		p->trflag[1] = false;
		primaryUserArr[rid].trflag[1] = false;
		primaryUserArr[rid].trflag[0] = false;

		p->getChannel()->unlockChannel();// release channel
		p->setBusyFlag(0);

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
		this->getChannel()->set_history(0);
	}

	// once the channel is fully used by Pu. set to '0'. So that others can use.
	this->getChannel()->setChReq(0);
	printf("PU %d RELEASED CHANNEL %d AND DATA TXN STOPS !!\n",this->nodeId,this->ch->getChId());		
}

/*
Constructor: Initializes the object to default values
 */
secondary::secondary() {
	nodeId = 0;
	busyFlag = 0;
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
	busyFlag = 0;
	ch = NULL;
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

/*---------------------------------------------------
  Implementation of PO-MAC

  Modifying the Master slave to suit distributed.
  1.Copy the full code of MS here.
  2.Now make sure you predict channel possibility as per MS-FPFC.
  3.Now what should be done is, stop before iterating whether each channel is free.
  4.Select a channel that is lock-free and prediction is 1.
  5.Last point is how often to run MPC ? 
  ---------------------------------------------------*/
/*
   Brief - It is a SU task. A task is created for each SU. And each SU executes the functionality in the below routine.

*/

void* secondary::thread_maker(void* data) 
{

	secondary *s = static_cast<secondary*>(data);

	int count, nextIndex;
	struct timeval start;
	struct timeval end;
	struct timeval mstart;
	struct timeval mend;
	int i=0;
	int a=0;
	FILE *fp = NULL;
	FILE *fp1= NULL;
	int read[80];
	double runTime = 0.0;
	int rid = 1;
	int free_channel[NUM_CHANNELS];
	int selected_channel = 0;
	turn = 0;
	scanTime=0.0;

	for(i =0; i < max_secusers; i++)
	{
		suregistry[i] == 99;
		su_chan[i]= 99;
	}
	for(int i=0;i < NUM_CHANNELS ;i++)
		free_channel[i] = 99;

	/*
	   For scalable network simulation.
	   Control number of SU's active, using a variable array allowed_index;
	 */

	sleep(1);
	while(1 && (exitflag == 0)) {

		s->setTxtime(Exponential(5) + s->getTxtime() );

		rid = (unsigned)rand()%max_secusers;

		while( s->getTxtime() >= 0.0 &&(exitflag == 0)) {

		/***************************************Manager SU*******************************************************************/
			if(turn == s->getId())
			{
				usleep(0.30478*1e6);
				s->runtime += 0.30478;

				count = 0;
				//		counter[s->getId()] = 1;
				int sum = 0;
				gettimeofday(&mend, NULL);
				//printf("Time elapsed = %f\n", (double) (mend.tv_sec - sysstart.tv_sec ) );

				/*		now on getting a msg from su, increment Ns.
						how to send msg to MSU, by taking a lock over a variable and setting the su number in the variable
						now give a index number to su

				 */

				gettimeofday(&start, NULL);
				pthread_mutex_lock(&QMutex);
				Ns = 0;

				for(i = 0; i< ( max_secusers); i++)
				{
					if(suregistry[i] == 1)
					{
						Ns += suregistry[i];
						su_chan[i] = i%NUM_CHANNELS; //allocate a PU channel to be sensed by SU 
						//			printf("msu registration done for %d\n",i);
					}
				}

				pthread_mutex_unlock(&QMutex);
				gettimeofday(&end, NULL);
				runTime = (end.tv_sec + (end.tv_usec/1000000.0)) - (start.tv_sec + (start.tv_usec/1000000.0));

				scanTime += runTime;
			}



			/***************************************Normal SU*************************************************************/

			gettimeofday(&start, NULL);

			int local_free_channel[NUM_CHANNELS];
			int selected_channel = 0;
			int cflag = 0;
			int channel_sense = 1;

			s->setRxdist();//---------------------
			int xcount = 0;
			int count = 0;

			//			printf(" registeration by NSU %d\n",s->getId());
			pthread_mutex_lock(&QMutex);
			suregistry[s->getId()]=1;
			pthread_mutex_unlock(&QMutex);

			//next index is to be given by MSU

			if( (su_chan[s->getId()] != 99 )  ){// wait till Master allocates a index number

				pthread_mutex_lock(&QMutex);

				nextIndex = su_chan[s->getId()];//nextIndex is assigned by master

				//since index is obtained from master, reset su_chan[id] to 99. so as the array is readied for next iteration
				su_chan[s->getId()] = 99;

				pthread_mutex_unlock(&QMutex);

				//calculate the channel to sense
				channel_sense = nextIndex;

				do{
					cflag = 1;

					if(turn== s->getId())
					{
						//		turn = (turn + 1)%max_secusers;
					}

					if( exitflag == 1){
						break;
					}

					pthread_mutex_lock(&FMutex);
					if( (chArray + channel_sense)->getChReq()==0 ) { 
						usleep(SENSING_TIME);

						free_channel[channel_sense] = 1;
					}
					else
					{
						usleep(SENSING_TIME);
						free_channel[channel_sense] = 0;
					}
					pthread_mutex_unlock(&FMutex);
					count = 0;

					//prepare a list of free channel. remember every SU, updates the "free_channel" list
					for(int i=0;i < NUM_CHANNELS ;i++)
					{
						//pthread_mutex_lock(&FMutex);
						//Actually the free_channel array is indexed by PU  channel number
						if(free_channel[i] == 1)//channel free
						{
							local_free_channel[count] = i; //local_free_channel has the list of free channels from all users
							count++;
						}
						//pthread_mutex_unlock(&FMutex);
					}

					usleep(50); //DIFS time
					usleep( (rand()%(max_secusers))*10e2);		//wait for some random time. ideally less than .3048

					//randomly choose a channel from list of available free channels(count)
					if(count > 0)
						selected_channel = local_free_channel[rand()%count];
					else
						selected_channel = rand()%NUM_CHANNELS;

					while(s->trflag[1] == true && (exitflag == 0)){
						//			printf("\n  Transmitter is %d blocked as receiver flag set\n", s->getId() );
					}
					//The selected PU channel is preempted for the sender and receiver SUs	
					//						printf("SU %d selected channel %d \n", s->getId(), selected_channel );
					//if the receiver is busy, wait
					while( (secondUserArr[rid].trflag[0] == true || secondUserArr[rid].trflag[1] == true) && (exitflag == 0));


					if(free_channel[selected_channel] == 1)
					{
						pthread_mutex_lock(&FMutex);
						free_channel[selected_channel] = 0;
						pthread_mutex_unlock(&FMutex);


						//wait for one time slot and send data
						usleep(0.14228*1e6);

						if(chArray[selected_channel].lockChannel() == 1) 
						{// no channel req, so send data

							if(turn== s->getId())
							{
								turn = (turn + 1)%max_secusers;
							}
							s->setBusyFlag(1); 
							s->setChannel(&chArray[selected_channel]); 

							s->trflag[0] = true;
							s->trflag[1] = false;

							secondUserArr[rid].trflag[1] = true;
							secondUserArr[rid].trflag[0] = false;

							s->getChannel()->userid = s->getId();

							s->txData();

							s->trflag[0] = false;
							s->trflag[1] = false;

							secondUserArr[rid].trflag[1] = false;
							secondUserArr[rid].trflag[0] = false;

							s->setBusyFlag(0);
							s->getChannel()->userid = 99;
							s->getChannel()->unlockChannel();
							break;
						}
						cflag = 0;
					}
					s->failure++;
				}while(cflag == 1 && (exitflag == 0));

			}

			gettimeofday(&end, NULL);
			runTime = (end.tv_sec + (end.tv_usec/1000000.0)) - (start.tv_sec + (start.tv_usec/1000000.0));	
			s->runtime += runTime;
		}

		double slt = Exponential(5);
		sleep(slt);
		s->freetime += slt;
	}	
	//printf(" SU %d free time-> %f\n",s->getId(),s->freetime);
	pthread_exit(NULL);
}

/*
   Brief- The function does transmission of SU data.
   The data is transmitted for time "getTxtime()"

   Parameters affected - incNumOfInterference, incNumOfBackoffs, busytime
*/

void secondary::txData( )
{
	usleep(SENSING_TIME);
	printf("SU %d ACQUIRED CHANNEL %d AND DATA TXN STARTS !!\n",this->nodeId,this->ch->getChId());

	float cutoffdist;//-----------
	cutoffdist = pow(10,(((this->getChannel()->getPTxPwr())+2+90)/20))/(41.88*(this->getChannel()->chFreq));//------------

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

		counter[this->getId()] = 0;
	}
	this->setcompletedTx() ;
	printf("SU %d RELEASES CHANNEL %d AND DATA TXN STOPS !!\n",this->nodeId,this->ch->getChId());
	//	printf("\n%d\'s TRANSMISSION ENDED with %d \n",this->getId(), this->ch->getChId());
}
