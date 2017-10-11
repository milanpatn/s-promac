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
Brief: This function calls the MPC and at that time all the other threads are under 
suspension. The variable counter is used to acheive this
*/
void x()
{
	int status;
	int pid = vfork();

	if(pid == 0)
	{
		//child
		//char *new_argv[2];
		//new_argv[0]="./sh.sh";
		//new_argv[1]=0;
		//execvp(new_argv[0],new_argv);	
	}
	else{
		//parent
		//Fwhile(	wait(&status) != pid);//wait for child to complete
	}
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
		while(p->trflag[1] == true && (exitflag == 0))
		{
			while(flag== 0)
			{
				counter[p->getId() - 100 + 1 + NUM_SEC_USERS ] = 1;
			}
			counter[p->getId() - 100 + 1 + NUM_SEC_USERS ] = 0;
		}

		rid = (unsigned)rand()%NUM_PRIMARY_USERS;	

		//if the receiver is busy, wait
		while( (primaryUserArr[rid].trflag[0] == true || primaryUserArr[rid].trflag[1] == true) && (exitflag == 0))
		{
			while(flag== 0)
			{
				counter[p->getId() - 100 + 1 + NUM_SEC_USERS ] = 1;
			}
			counter[p->getId() - 100 + 1 + NUM_SEC_USERS ] = 0;
		}

		gettimeofday(&start, NULL);

		while(!p->getChannel()->lockChannel()) { //trying to lock the allocated channel

			p->getChannel()->setChReq(1);//req for channel 

			if(exitflag == 1)
				break;

			usleep(BACKOFF_TIME);//waiting
			while(flag== 0)
			{
				counter[p->getId() - 100 + 1 + NUM_SEC_USERS ] = 1;
			}
			counter[p->getId() - 100 + 1 + NUM_SEC_USERS ] = 0;


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
		while(flag== 0)
		{
			counter[p->getId() - 100 + 1 + NUM_SEC_USERS ] = 1;
		}
		counter[p->getId() - 100 + 1 + NUM_SEC_USERS ] = 0;

	
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

		while(flag== 0)
		{
			counter[this->getId() - 100 + 1 + NUM_SEC_USERS ] = 1;
		}
		counter[this->getId() - 100 + 1 + NUM_SEC_USERS ] = 0;

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
	int read1[NUM_CHANNELS];
	double sumtime = 0.0;
	struct timeval start,end;
	struct timeval sumstart,sumend;
	int rid = 0;

//	max_secusers = 10;

	struct timeval currentTime;
	gettimeofday(&currentTime, NULL);

	if(rand()%3 == 0)
	{
		for(i=0;i<count;i++){
			read1[i]=rand()%1000;
		}
	}

	while(1 && (exitflag == 0)) {


		/**** Only for Sec User 1 ****/
		/**** start ****/
		if( s->getId() == turn &&  (exitflag == 0) )//every 5th sec run mpc in SU 1
		{
			//start prediction
			count = 0;
			counter[s->getId() ] = 1;
			//turn == rand()%max_secusers;
			int read[NUM_CHANNELS];
			int histcount = 0;
	
			flag=0;
			int sum = 0;
			sleep(0.30478);
			s->runtime += 0.30478;

			pthread_mutex_lock(&QMutex);

			gettimeofday(&start, NULL);

			do{
				sum = 0;
				for(i = 0; i< (NUM_PRIMARY_USERS + NUM_SEC_USERS); i++)
				{
					sum += counter[i];
				}
				if(exitflag == 1)
					break;
				//printf("sum = %d\n",sum);		
			}while(sum <(NUM_PRIMARY_USERS + NUM_SEC_USERS - 1) );



			struct Qnode* temp = s->channelList->getFirst();	

			fp1 = fopen("../matlab/num2.txt","w");//channel history file
			/*
			   1.Iterate through each channel.
			   2.save the history in to num2.txt
			 */
			for( i = 0; i < NUM_CHANNELS; i++ )
			{
				read[i]=0;
			}

			count = 0;
			nextIndex = 0;
			while(count < s->channelList->count()&& temp!=NULL ){

				nextIndex = s->channelList->getValue(temp);
#ifdef DEBUG
				//		printf("SU = %d nextIndex = %d\n",s->getId(),nextIndex);
#endif
				struct dlnode *temp1 = (chArray + nextIndex)->start;

				for(int j=0;j < 10 && (temp1 != NULL);j++) //writing channel's history in file
				{
					fprintf(fp1,"%d ",temp1->data);
					// add the distance of each PU from the SU
					temp1 = temp1->next;
					histcount++;
				}
				fprintf(fp1,"\n");

				temp = temp->next;
				count++;
			} 

			fclose(fp1);

			//						printf("histcount = %d\n",histcount);
			if( (histcount/NUM_CHANNELS) >9 )// if enough history then predict
			{
				temp = s->channelList->getFirst();	

				gettimeofday(&mstart, NULL);
				//matlab prediction part		
				//x();
				gettimeofday(&mend, NULL);
				//printf("current iterataion matlab Time  = %f\n", (double) (mend.tv_sec - mstart.tv_sec) );
				matlab_time += (double) (mend.tv_sec - mstart.tv_sec );

				fp = fopen("../matlab/op.txt","r");

				count = 0;
				while(fscanf(fp,"%d",&a) != EOF)//store prediction results of each channel in array
				{
					read[count]=a;
					count++;
				}
				fclose(fp);

				//printf("\n prediction results: ");
				for(int i=0; i < count;i++)
				{
					//printf(" %d",read[i]);
				}
				//printf("\n ");

				gettimeofday(&mend, NULL);

				count = 0;
				nextIndex = s->channelList->getValue(temp);
				/*
				   while(count < s->channelList->count()&& temp!=NULL )
				   {
				   nextIndex = s->channelList->getValue(temp);
				//printf("busy period of %d - %f\t",nextIndex,(chArray + nextIndex)->getBusyPeriod()); 
				//printf("Free period of %d - %f\n",nextIndex,(chArray + nextIndex)->getFreePeriod()); 
				temp = temp->next;
				count++;
				}
				 */

			}
			/*
			   1.Elements history in file num2.txt is in FIFO
			   2.read array has prediction's as in abv FIFO order.
			   3.Therefore, use linear search in read array to make decisions
			   4. Add the distance as a parameter.

			 */

			count = 0;
			temp = s->channelList->getFirst();	

			do {
				nextIndex = s->channelList->getValue(temp);

				if( read[count] == 1 )// if true unlock channel else push back to the queue
				{
					temp = temp->next ;
					count++;

				} else {
					s->channelList->removeNode(nextIndex);
					s->channelList->enqueue(nextIndex);
					temp = temp->next;
					count++;
				}

			}while(count < s->channelList->count()&& temp!=NULL );


			gettimeofday(&end, NULL);
			double runTime = (end.tv_sec + (end.tv_usec/1000000.0)) - (start.tv_sec + (start.tv_usec/1000000.0));
			scanTime += runTime;

			flag = 1;
			gettimeofday(&mend, NULL);

			//printf("scanTime = %f\n", scanTime );
			//printf("Time elapsed = %f\n", (double) (mend.tv_sec - currentTime.tv_sec ) );

			pthread_mutex_unlock(&QMutex);

			sleep(0.30478);
			s->runtime += 0.30478;
			//end prediction 
		}

				turn = 99;
		/**** end ****/
		int k=0;
		rid = (unsigned) rand()%NUM_SEC_USERS;
		s->setTxtime(Exponential(5));
		s->setRxdist();//---------------------

		gettimeofday(&start, NULL);
		while(s->getTxtime() >= 0.0 && (exitflag == 0)) {

			usleep(BACKOFF_TIME);

			while( (secondUserArr[rid].trflag[0] == true || secondUserArr[rid].trflag[1] == true) && (exitflag == 0))
			{
				while(flag== 0)
				{
					counter[s->getId()] = 1;
				}
				counter[s->getId()] = 0;
				usleep(10);
			}

			while( (s->trflag[1] == true) && (exitflag == 0)){

				while(flag== 0)
				{
					counter[s->getId()] = 1;
				}
				counter[s->getId()] = 0;
			}


			int nextIndex = 0;
			struct Qnode* temp;

			if(flag == 1)
				temp = s->channelList->getFirst();

			//printf("SU = %d logging in\n",s->getId());

			for(int i=0;i <  s->channelList->count() && temp!=NULL && flag == 1;i++)
			{
				nextIndex = s->channelList->getValue(temp);
				temp = temp->next;

				if(nextIndex >= 0 && nextIndex < NUM_CHANNELS)
				{

					if( (chArray + nextIndex)->getChReq() == 0 && (read1[nextIndex] < 700) ){

						if( ((chArray + nextIndex)->lockChannel() == 1) )
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

						}  
					}
				}else{
					temp = s->channelList->getFirst();
					continue;
				}
			}


			while(flag ==  0)
			{
				counter[s->getId() ] = 1;
			}
			counter[s->getId() ] = 0;

		}
		gettimeofday(&end, NULL);
		double runTime = (end.tv_sec + (end.tv_usec/1000000.0)) - (start.tv_sec + (start.tv_usec/1000000.0));	
		s->runtime += runTime;

		if(rand()%3 == 0)
		{
			for(i=0;i<count;i++){
				read1[i]=rand()%1000;
			}
		}

		turn == rand()%NUM_SEC_USERS;
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
		while(flag== 0)
		{
			counter[this->getId()] = 1;
		}
		counter[this->getId()] = 0;
	

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
