#include "node.h"
#include<unistd.h>
using namespace std;
int exitflag = 0;
extern secondary secondUserArr[NUM_SEC_USERS];
extern primary primaryUserArr[NUM_PRIMARY_USERS];
double matlab_time = 0.0;
double scanTime=0.0;
channel *chArray = NULL;
queue secondaryChList;
pthread_mutex_t QMutex;
struct timeval sysstart;
int counter[(NUM_PRIMARY_USERS + NUM_SEC_USERS)];
extern int turn;
struct timeval simStart;
unsigned int  startTime = 0;
unsigned int curTime = 0;

int max_secusers=0;

int main(){
printf("OK");
	double localscantime = 0.0;
	double	ch_busy[NUM_CHANNELS];
	time_t curtime;
	struct tm *loctime;
	int begin, end;
	struct timeval currentTime;


	gettimeofday(&simStart, NULL);
	startTime = simStart.tv_sec ;	

	pthread_mutex_init(&QMutex,NULL);
	srand(time(NULL));

	chArray = (channel*)malloc(NUM_CHANNELS * sizeof(channel));

	memset(chArray, '\0', NUM_CHANNELS*sizeof(channel));

	scanTime=0.0;
	matlab_time =0.0; 
	   max_secusers = NUM_SEC_USERS;
	turn = rand()%NUM_SEC_USERS;

/*
Initializing the PU channel
*/
	for(int i = 0; i < NUM_CHANNELS; i++)
	{
		chArray[i] = channel(i,(5150+(2.5*(rand() % 79)))); //assiging channel no and freq
		secondaryChList.enqueue(i);
	}

	for(int i=0; i< (NUM_PRIMARY_USERS + NUM_SEC_USERS);i++)
	{
		counter[i]=0;
	}

	pthread_t primaryUserThreads[NUM_PRIMARY_USERS];

	
/*
Initializing the PU
*/
	
	for(int i = 0; i < NUM_PRIMARY_USERS; i++)
	{
		primaryUserArr[i].setId(100 + i);
		primaryUserArr[i].setChannel(&chArray[(i%NUM_CHANNELS)]); //Each Prim user is allocated a channel
		pthread_create(&primaryUserThreads[i],NULL,&primary::thread_maker,static_cast<void*>(&primaryUserArr[i]));
	}

	pthread_t secondUserThreads[NUM_SEC_USERS];

/*
Initializing the SU 
*/
	for(int i = 0; i < NUM_SEC_USERS; i++)
	{
		secondUserArr[i].setId( i);
		secondUserArr[i].setChannelList(&secondaryChList);
		pthread_create(&secondUserThreads[i],NULL,&secondary::thread_maker,static_cast<void*>(&secondUserArr[i]));
		//printf("thread %u started\n",i);
	}
	int turncount=0;
/*
The begin, end & max_secusers are used here to make scalable simulation automatic.
It can be scaled for any number, by manipulating the begin and end variables
*/  
	begin = 0;
	end = 10;
	max_secusers = 20;
	//max_secusers = end;	

/*
To Be used in case of Scalability simulation - Start
*/
/*

	   begin = 0;
	   end = 10;
	   max_secusers = end;

	   while(begin <= 40)
	   {

	   for(int i = begin; i < end; i++)
	   {
	   secondUserArr[i].setId( i);
			secondUserArr[i].setChannelList(&secondaryChList);
			pthread_create(&secondUserThreads[i],NULL,&secondary::thread_maker,static_cast<void*>(&secondUserArr[i]));
			printf("thread %u started\t max_secusers = %d\n",i,max_secusers);
		}


		for(int i=0; i <200; i++)
		{
			if( (turn == 99) && turncount >= ( rand()%max_secusers)  )
			{
				turn = rand()%max_secusers;
				turncount = 0;
			}
			turncount++;

			sleep(4);
		}

		//	sleep(1000);


		printf("\n");
		printf("Scan Time  %lf\n", scanTime);

		for(int i = 0; i < NUM_CHANNELS; i++){
			ch_busy[i]= chArray[i].getBusyPeriod();	
		} 

		gettimeofday(&currentTime, NULL);
		curTime = currentTime.tv_sec ;
		printf("Num of Channels = %d\n",NUM_CHANNELS );
		printf("Num of PU = %d\n",NUM_PRIMARY_USERS );
		printf("Num of SU = %d\n",max_secusers );
		printf("simtime = %d\n",(curTime - startTime  ));

		for(int i = 0;i < NUM_CHANNELS;i++) {
			printf("Busy time  %f  \t", chArray[i].getBusyPeriod());
			printf("Free time  %f  \n", chArray[i].getFreePeriod());
		}

		double px = 0.0;

		for(int i = 0;i < NUM_PRIMARY_USERS;i++) {
			px += primaryUserArr[i].busytime ;
		}
		printf("Avg busyTime of Primary  = %f\n", px/NUM_PRIMARY_USERS);

		double sbtime = 0.0;

		for(int i = 0;i < max_secusers;i++) {
			sbtime += secondUserArr[i].busytime ;
		}
		printf("Avg busyTime of secondary  = %f\n", sbtime/max_secusers);

		double sftime = 0.0;

		for(int i = 0;i < max_secusers;i++) {
			sftime += (secondUserArr[i].freetime );
		}
		printf(" Avg freetime of secondary  = %f\n", sftime/max_secusers);

		double avg = 0.0;

		for(int i = 0;i < NUM_CHANNELS;i++) {

			avg +=  chArray[i].getBusyPeriod();
		}
		avg= avg/NUM_CHANNELS;
		printf("Channel Utilisation  %f \n", (avg/((curTime - startTime  )- scanTime ))*100);

		int bk = 0;

		for(int i = 0;i < max_secusers;i++) {
			bk += secondUserArr[i].getNumOfBackoffs();
		}
		printf(" Avg Number of Backoff's = %d\n",bk/max_secusers);

		int rts = 0;
		for(int i = 0;i < max_secusers;i++) {
			rts += secondUserArr[i].runtime ;
		}
		printf(" Runtime of Secondary = %d\n",rts/max_secusers);

		max_secusers += 10;
		begin = begin + 10;
		end = end + 10;
	}

*/	

/* End */
	
	while(1)
	{
		gettimeofday(&currentTime, NULL);
		curTime = currentTime.tv_sec ;

		max_secusers = NUM_SEC_USERS;

		if(  turncount >= 1000*( rand()%NUM_SEC_USERS)  )
		{
			turn = rand()%NUM_SEC_USERS;
			turncount = 0;
			sleep(1);
		}
		turncount++;
//			printf("\t\t\t\telapsed Time = %d\n", curTime - startTime);
		if(  (curTime - startTime) > (SIM_TIME ))
		{
			exitflag = 1;	

			for(int i = 0; i < NUM_SEC_USERS; i++)
			{
				pthread_join(secondUserThreads[i],NULL);
				//			printf( "sec thread %d joined", i);	
			}


			for(int i = 0; i < NUM_PRIMARY_USERS; i++)
			{
				pthread_join(primaryUserThreads[i],NULL);
				//			printf( "prim thread %d joined", i);			
			}
			localscantime = scanTime;

			printf("Scan Time  %lf\n", localscantime);

			for(int i = 0; i < NUM_CHANNELS; i++){
				ch_busy[i]= chArray[i].getBusyPeriod();	
			} 

			pthread_mutex_destroy(&QMutex);

			gettimeofday(&currentTime, NULL);
			curTime = currentTime.tv_sec + (currentTime.tv_usec/1000000.0);
			printf("Num of Channels = %d\n",NUM_CHANNELS );
			printf("Num of PU = %d\n",NUM_PRIMARY_USERS );
			printf("Num of SU = %d\n",NUM_SEC_USERS );
			printf("simtime = %d\n",(curTime - startTime  ));

			for(int i = 0;i < NUM_CHANNELS;i++) {
				printf("Busy time  %f  \t", chArray[i].getBusyPeriod());
				printf("Free time  %f  \n", chArray[i].getFreePeriod());
			}

			double px = 0.0;

			for(int i = 0;i < NUM_PRIMARY_USERS;i++) {
				px += primaryUserArr[i].busytime ;
			}
			printf("Avg busyTime of Primary  = %f\n", px/NUM_PRIMARY_USERS);

			double sbtime = 0.0;

			for(int i = 0;i < NUM_SEC_USERS;i++) {
				sbtime += secondUserArr[i].busytime ;
			}
			printf("Avg busyTime of secondary  = %f\n", sbtime/NUM_SEC_USERS);

			double sftime = 0.0;

			for(int i = 0;i < NUM_SEC_USERS;i++) {
				sftime += (secondUserArr[i].freetime );
			}
			printf(" Avg freetime of secondary  = %f\n", sftime/NUM_SEC_USERS);

			double avg = 0.0;

			for(int i = 0;i < NUM_CHANNELS;i++) {

				avg +=  chArray[i].getBusyPeriod();
			}
			avg= avg/NUM_CHANNELS;
			printf("Channel Utilisation  %f \n", (avg/(curTime - startTime  - scanTime ))*100);

			int bk = 0;

			for(int i = 0;i < NUM_SEC_USERS;i++) {
				bk += secondUserArr[i].getNumOfBackoffs();
			}
			printf(" Avg Number of Backoff's = %d\n",bk/NUM_SEC_USERS);

			int rts = 0;
			for(int i = 0;i < NUM_SEC_USERS;i++) {
				rts += secondUserArr[i].runtime ;
			}
			printf(" Runtime of Secondary = %d\n",rts/NUM_SEC_USERS);



//			printf(" %f\n", (rts - sbtime - scanTime));
//			printf(" %f",bk/sbtime );

			/* Get the current time. */
			curtime = time (NULL);
			/* Convert it to local time representation. */
			loctime = localtime (&curtime);			

			/* Print out the date and time in the standard format. */
			fputs (asctime (loctime), stdout);
			printf("\n");
			free(chArray);
			return 0;	

		}
	}

}

