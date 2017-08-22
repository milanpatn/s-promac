#include "node.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

using namespace std;
extern channel* chArray;
extern pthread_mutex_t QMutex;
extern pthread_cond_t QCond;
int flag=1;
extern int exitflag;
extern double matlab_time;
extern double scanTime;
secondary secondUserArr[NUM_SEC_USERS];// list of SU users
primary primaryUserArr[NUM_PRIMARY_USERS];// list of PU users
extern int counter[(NUM_PRIMARY_USERS + NUM_SEC_USERS)]; //Used to bring all task to suspension
secondary secMaster;
extern struct timeval simStart;
/*
Brief: This function calls the MPC and at that time all the other threads are under 
suspension. The variabl counter is used to acheive this
*/
void x()
{
	int status;

	int pid = vfork();

	if(pid == 0)
	{
		//child
		char *new_argv[2];
		new_argv[0]="./sh.sh";
		new_argv[1]=0;
		execvp(new_argv[0],new_argv);	
	}
	else{
		//parent
		while(	wait(&status) != pid);//wait for child to complete
	}
}

/*
   Brief - A timer value generator. 
   Used for exponential Backoff
 */

double Exponential(double mean)
{
	double R;
	R = (double)rand()/(double)(RAND_MAX);
	return  -mean*log(R);

}

void node::setRxdist() {//--------------------------------------------------
	//Rxdist = 0.85+(rand() % 1);
	Rxdist = Exponential(0.3);
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

void node::setTxpwr(float p) {//---------------------------------

	Txpwr = p;
}

void node::setOpttxpwr(float pwr){//-------------------------
	Opttxpwr = pwr;
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
	/*	sprintf(ta,"%d\n",fti);
		fputs(ta,fp);

		fputs("pattern - ",fp);

		for(i=0;i<Ft/.30478;i++)
		{
		fputs("0 ",fp);
		p->getChannel()->pattern[i]=0;
		}
		int k=(Ft/.30478);
		for(i=0;i<Fi/.30478;i++)
		{
		p->getChannel()->pattern[i+k]=1;
		sprintf(ta,"%d ",p->getChannel()->pattern[i+k]);
		fputs(ta,fp);
		}
		p->getChannel()->patterncount = Ft/.30478 + Fi/.30478;
		fclose(fp);
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

		while(p->trflag[1] == true && (exitflag == 0))
		{
			while(flag== 0)
			{
				counter[p->getId() - 100 + 1 + 20 ] = 1;
			}
			counter[p->getId() - 100 + 1 + 20 ] = 0;
		}

		rid = (unsigned)rand()%NUM_PRIMARY_USERS;	

		//if the receiver is busy, wait
		while( (primaryUserArr[rid].trflag[0] == true || primaryUserArr[rid].trflag[1] == true) && (exitflag == 0))
		{
			while(flag== 0)
			{
				counter[p->getId() - 100 + 1 + 20 ] = 1;
			}
			counter[p->getId() - 100 + 1 + 20 ] = 0;

		}



		while(!p->getChannel()->lockChannel()) { //trying to lock the allocated channel

			p->getChannel()->setChReq(1);//req for channel 

			//			printf(" channel no: %d  request made by PU %d held by SU %d\n", p->getChannel()->getChId(), p->getId() , p->getChannel()->userid);

			if(exitflag == 1)
				break;

			usleep(BACKOFF_TIME);//waiting

			while(flag== 0)
			{
				counter[p->getId() - 100 + 1 + 20 ] = 1;
			}
			counter[p->getId() - 100 + 1 + 20 ] = 0;
		}

		/*Busy Time*/
		if( i> 3  ) // every 2 cycles randomly select one patter
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
						fputs("0 ",fp);
						p->getChannel()->pattern[i]=0;
						}
						k=(Ft/.30478);
						for(i=0;i<Fi/.30478;i++)
						{
						p->getChannel()->pattern[i+k]=1;
						sprintf(ta,"%d ",p->getChannel()->pattern[i+k]);
						fputs(ta,fp);
						}
						p->getChannel()->patterncount = Ft/.30478 + Fi/.30478;
						fclose(fp);
			 */			i=0;
		}
		i++;

		gettimeofday(&start, NULL);

		p->trflag[0] = true;
		p->trflag[1] = false;

		primaryUserArr[rid].trflag[1] = true;
		primaryUserArr[rid].trflag[0] = false;

		p->setBusyFlag(1);// set busy flag


		p->txData( );// send data
		
		p->trflag[0] = false;
		p->trflag[1] = false;
		primaryUserArr[rid].trflag[1] = false;
		primaryUserArr[rid].trflag[0] = false;

		p->setBusyFlag(0);
		p->getChannel()->unlockChannel();// release channel

			p->getChannel()->setChReq(0);//req for channel 
		gettimeofday(&end, NULL);

		double runTime = (end.tv_sec + (end.tv_usec/1000000.0)) - (start.tv_sec + (start.tv_usec/1000000.0));	


		//	double s = Exponential(8);
		//	sleep(s);
		unsigned  s = Fi/0.30478; //0.060956*5 since ft & fi return result multiplied by 5

		/*Idle times*/
		for(j=0;j<s ;j++)
		{

			//channel currently not under transmission, so available
			p->getChannel()->set_history(1);
		//	printf("channel %d free - %d\n", p->getChannel()->getChId(),p->getChannel()->get_history()->data);

			if(exitflag == 1)
				break;

			usleep( 0.30478*1e6 ); 
			//p->getChannel()->setFreePeriod(p->getChannel()->getFreePeriod() + (double)0.30478);

			p->freetime += (double)0.30478;
			while(flag== 0)
			{
				counter[p->getId() - 100 + 1 + 20 ] = 1;
			}
			counter[p->getId() - 100 + 1 + 20 ] = 0;
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

/*	
	//FOR SSDF ATTACK STARTS
	int attack = 0;
	attack = this->nodeId;

	
	if(attack < 102){//ATTACK
		printf("*******The %d Attacker transmission !!********\n", attack);
		//Fi = 0;
		//Ft = 10000;
		s = 100000;
	}
	// FOR SSDF ATTACK ENDS

*/

	for(int j=0;j<s  ;j++)
	{
		//channel currently under transmission, so not available
		this->getChannel()->userid = this->getId();
		this->getChannel()->set_history(0);

		if(exitflag == 0){
			usleep( 0.30478*1e6 ); 
		}		else
		{
			break;
		}

		this->busytime += (double) 0.30478;
		this->getChannel()->setBusyPeriod(this->getChannel()->getBusyPeriod() + (double) 0.30478); //busy time range
		//		printf("\n user %d  busy on channel: %d - %f's \n", this->getId(),this->ch->getChId(),this->getChannel()->getBusyPeriod());
		while(flag== 0)
		{
			counter[this->getId() - 100 + 1 + 20 ] = 1;
		}
		counter[this->getId() - 100 + 1 + 20 ] = 0;
	}

// once the channel is fully used by Pu. set to '0'. So that others can use.
	
	this->getChannel()->userid = 99;
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

void* secondary::masterThread(void* data) {
	secondary *m = static_cast<secondary*>(data);
	int count, nextIndex;
	struct timeval mstart;
	struct timeval mend;
	struct timeval end;
	struct timeval start;
	struct timeval sumstart;
	struct timeval sumend;
	double curTime = 0.0;
	int i=0;
	int a=0;
	FILE *fp = NULL;
	FILE *fp1= NULL;
	int read[80];
	double sumtime = 0.0;

	while(1  && (exitflag == 0) ) {

		double chutil = 0.0;
		double btime = 0.0;
		double ftime = 0.0;


		flag=0;
		pthread_mutex_lock(&QMutex);

		gettimeofday(&start, NULL);

		int sum = 0;
		int read[NUM_CHANNELS];
		int histcount = 0;
		count = 0;
		nextIndex = 0;

		//		gettimeofday(&sumstart, NULL);

		do{
			sum = 0;
			for(i = 0; i< (NUM_PRIMARY_USERS + 20); i++)
			{
				sum += counter[i];
			}
//		printf("sum  = %d\n", sum );
			if(exitflag == 1)
				break;
		}while(sum <(NUM_PRIMARY_USERS + 20 - 1) );

		//		gettimeofday(&sumend, NULL);

//		double stime =  (double) ( (sumend.tv_sec + (sumend.tv_usec/1000000.0) )- (sumstart.tv_sec + (sumstart.tv_usec/1000000.0) ) );

//		sumtime += stime;


//		if(stime > (0.30478))
		//	scanTime += stime;



		struct Qnode* temp = m->channelList->getFirst();	

		fp1 = fopen("../../matlab/num2.txt","w");//channel history file
		/*
		   1.Iterate through each channel.
		   2.save the history in to num2.txt
		 */
		while(count < m->channelList->count()&& temp!=NULL ){

			nextIndex = m->channelList->getValue(temp);

			struct dlnode *temp1 = (chArray + nextIndex)->start;

			for(int j=0;j < 10 && (temp1 != NULL);j++) //writing channel's history in file
			{
				fprintf(fp1,"%d ",temp1->data);
				temp1 = temp1->next;
				histcount++;
			}

			fprintf(fp1,"\n");

			temp = temp->next;
			count++;

		} 

		fclose(fp1);

		if( (histcount/NUM_CHANNELS) >9 )// if enough history then predict
		{

			temp = m->channelList->getFirst();	

			gettimeofday(&mstart, NULL);
			//matlab prediction part		
			x();
			gettimeofday(&mend, NULL);
			matlab_time +=(mend.tv_sec + (mend.tv_usec/1000000.0)) - (mstart.tv_sec + (mstart.tv_usec/1000000.0));
			//printf("current iterataion matlab Time  = %f\n", (double) (mend.tv_sec - mstart.tv_sec) );


			fp = fopen("../../matlab/op.txt","r");

			count = 0;
			while(fscanf(fp,"%d",&a) != EOF)//store prediction results of each channel in array
			{
				read[count]=a;
				count++;
			}
			fclose(fp);

//			printf("\n prediction results: ");
		//	for(int i=0; i < count;i++)
//			{
//				printf(" %d",read[i]);
		//	}
//			printf("\n ");

/*
			count = 0;
			nextIndex = m->channelList->getValue(temp);
			btime = 0.0;
			while(count < m->channelList->count()&& temp!=NULL )
			{
				nextIndex = m->channelList->getValue(temp);
				btime += (chArray + nextIndex)->getBusyPeriod();
				ftime += (chArray + nextIndex)->getFreePeriod();
				temp = temp->next;
				count++;
			}
			printf(" Avg busy period - %f\n", btime/10); 
			printf(" Avg Free period - %f\n", ftime/10); 
*/
		}

		/*
		   1.Iterate to find which channel is free
		   2.For the free channel, apply prediction
		 */

		count = 0;
		temp = m->channelList->getFirst();	

		do {
			nextIndex = m->channelList->getValue(temp);

			if(( read[count] == 0 ) ) {

				m->channelList->removeNode(nextIndex);
				m->channelList->enqueue(nextIndex);
				temp = temp->next ;
				count++;

			} else {

				temp = temp->next;
				count++;
			}
				 (chArray + nextIndex)->pudist == rand()%1000;

		}while(count < m->channelList->count()&& temp!=NULL );


		//	curTime = end.tv_sec + (end.tv_usec/1000000.0);


		//		printf("Scan Time = %f\n", scanTime );
		//		printf(" matlab Time  = %f\n",  matlab_time);
		//		printf(" Avg Channel utilisation - %f\n\n", (btime*10)/( elapsed - scanTime )); 

		gettimeofday(&end, NULL);
		double runTime = (end.tv_sec + (end.tv_usec/1000000.0)) - (start.tv_sec + (start.tv_usec/1000000.0));
		scanTime += runTime;

		pthread_mutex_unlock(&QMutex);
		flag = 1;

		double elapsed = (double) (end.tv_sec + (end.tv_usec/1000000.0) - (simStart.tv_sec + (simStart.tv_usec/1000000.0) ) ) ;
		//printf("Time elapsed = %f\n",elapsed );

		int i = 0;

		while(exitflag == 0 && (i < 10))
		{
			i++;
			sleep(1);
		}
	}
	//		printf("sum Time = %f\n", sumtime );
	pthread_exit(NULL);
}
/*
   Brief - It is a SU task. A task is created for each SU. And each SU executes the functionality in the below routine.

*/
void* secondary::thread_maker(void* data) {
	secondary *s = static_cast<secondary*>(data);
	double ss = Exponential(2);
	sleep(ss);
	int xcount = 0;
	int rid = 1;

	s->failure = 0;

	while(1  && (exitflag == 0) ) {
		struct timeval start, end;

		rid = (unsigned) rand()%20;	
		s->setTxtime(Exponential(5)); 


		while(s->getTxtime() >= 0.0 && (exitflag == 0)) {
			gettimeofday(&start, NULL);

			s->runtime += 0.40478;
			sleep(0.40478);

			while(flag== 0)
			{
				counter[s->getId() ] = 1;
			}
			counter[s->getId() ] = 0;

			while(s->trflag[1] == true && (exitflag == 0))
			{
				while(flag== 0)
				{
					counter[s->getId()] = 1;
				}
				counter[s->getId()] = 0;

			}

			while( (secondUserArr[rid].trflag[0] == true || secondUserArr[rid].trflag[1] == true) && (exitflag == 0)){
				while(flag== 0)
				{
					counter[s->getId()] = 1;
				}
				counter[s->getId()] = 0;
			}


			int nextIndex;
			struct Qnode* temp;


			temp = s->channelList->getFirst();

			usleep(BACKOFF_TIME);
			for(int i=0;i <  s->channelList->count() && temp!=NULL && flag==1;i++) {


				nextIndex = s->channelList->getValue(temp);
				temp = temp->next;

				if( (chArray + nextIndex)->getChReq() == 0 &&  (chArray + nextIndex)->pudist < 700){

					if( (chArray + nextIndex)->lockChannel() == 1  )
					{ 
						s->setBusyFlag(1); 
						s->trflag[0] = true;
						s->trflag[1] = false;

						secondUserArr[rid].trflag[1] = true;
						secondUserArr[rid].trflag[0] = false;

						s->setChannel(&chArray[nextIndex]); 

						s->txData();

						s->trflag[0] = false;
						s->trflag[1] = false;

						secondUserArr[rid].trflag[1] = false;
						secondUserArr[rid].trflag[0] = false;

						s->setBusyFlag(0);

						s->getChannel()->unlockChannel();

						break;				
					}
				}
			}

			while(flag== 0)
			{
				counter[s->getId() ] = 1;
			}
			counter[s->getId() ] = 0;


			s->failure++;

			gettimeofday(&end, NULL);
			double runTime = (end.tv_sec + (end.tv_usec/1000000.0)) - (start.tv_sec + (start.tv_usec/1000000.0));	
			s->runtime += runTime;
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


	while(this->getTxtime() >= 0.0 ) {		

		this->getChannel()->userid = this->getId();

		if(this->getChannel()->userid > 102){//ATTACK 

			if(this->ch->getChReq()==1 ) { 
				this->incNumOfBackoffs();
				printf("\n-----------------------------SU %d BACKING OFF FROM CHANNEL %d\n",this->nodeId,this->ch->getChId());
				return;
			}
		}//ATTACK

		if(exitflag == 0)
		{
			usleep(14.2e4);
		}		else
		{
			break;
		}

		this->getChannel()->setBusyPeriod(this->getChannel()->getBusyPeriod() + (double )0.142 );
		this->busytime += (double)  0.142;

		//printf("\n Sec user %d  busy on channel: %d - %f's \n", this->getId(),this->ch->getChId(),this->getChannel()->getBusyPeriod());

		this->Txtime = this->Txtime - (0.142);

		while(flag== 0)
		{
			counter[this->getId()] = 1;
		}
		counter[this->getId()] = 0;
	}
	printf("SU %d RELEASES CHANNEL %d AND DATA TXN STOPS !!\n",this->nodeId,this->ch->getChId());

	this->setcompletedTx() ;
}

