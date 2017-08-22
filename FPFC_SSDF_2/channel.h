#include <iostream>

#include"queue.h"
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <stdio.h>
#include <pthread.h>

extern queue secondaryChList;
using namespace std;

#define SENSING_TIME 1e5
#define AQUIRE_TIME 1
#define BACKOFF_TIME 1
#define NUM_CHANNELS 20
#define NUM_PRIMARY_USERS 10
#define NUM_SEC_USERS 20
#define SIM_TIME 10.00
#define PWR_THRESHOLD 0.5
#define HIST_SIZEE 10
#define PRED_RANGE 3

struct dlnode
{
	struct dlnode *prev;
	int data;
	struct dlnode *next;
};


class channel {
	private:	
		int chId;
		int busyFlag;
		int chReq;
		pthread_mutex_t  chMutex;
		double busyPeriod;
		double freePeriod;
		float pTxPwr;//-------------
		float optDist;//-------------
		int future[10];
		int count;

	public:		
		channel();
		channel(int id,double f);
		~channel();
		int userid;
		int pudist;
		int secchReq;
		struct dlnode *start,*end;
		int patterncount;
		int pattern[26];
		double chFreq;
		void setChId(int i);	
		void setChFreq(double f);
		void setBusyFlag(int b);
		void setChReq(int r);
		void setBusyPeriod(double p);
		void setPTxPwr(float pwr);//----------------------
		void setOptDist(float dist);//-----------

		int getChId();
		double getChFreq();
		int getBusyFlag();
		int getChReq();
		double getBusyPeriod();
		float getPTxPwr();//----------------------
		float getOptDist();//--------------
		int lockChannel();
		void unlockChannel();		
		struct dlnode* get_history();	
		void set_history(int data);
		void insert1(int data);
		struct dlnode* create(int data);
		void removee();
		void traverse();
		void traverse_pattern(int count);
		int* get_pred();	
		void set_pred(int data[10]);
		void setFreePeriod(double p) ;
		double getFreePeriod(); 

};
