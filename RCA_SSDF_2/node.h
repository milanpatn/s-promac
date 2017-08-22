/* 
 * File:  node.h
 * Author: nslab1
 *
 * Created on 24 November, 2014, 2:28 PM
 */

#ifndef NODE_H
#define	NODE_H

#include "channel.h"

class node {
	public: 
		int nodeId;
		int busyFlag;
		channel* ch;
		double Txtime;
		float Rxdist;//------------
		float Txpwr;//-----------------
		float Opttxpwr;//-----------------
		float Rxpwr;//--------------------
		double runtime;
		double busytime;
		double freetime;
		bool trflag[2];

		void setId(int id);
		void setBusyFlag(int f);
		void setChannel(channel* c);
		void setTxtime(double s);
		void setTxpwr(float p);//-------------------------
		void setRxdist();//-----------------------
		void setOpttxpwr(float pwr);//-------------------------
		//void setRxpwr(float rpwr);//-------------------------

		int getId();
		int getBusyFlag();
		channel* getChannel();
		double getTxtime();

		virtual void txData( ) = 0;
};

class primary : public node {
	public:
		primary();
		primary(int i, channel* c);
		~primary();


		void txData( );
		static void* thread_maker(void* data);
};

class secondary : public node {
	public:
		queue* channelList;
		int numOfBackoffs;
		int numOfFailures;//--------------------------------
		int numOfInterference;//--------------------------------
		int compTx;
		int  failure;
		secondary();
		secondary(int i, queue* c);
		~secondary();

		void setChannelList(queue* q);
		void incNumOfBackoffs();
		void incNumOfFailures();//----------------------------
		void incNumOfInterference();//----------------------------

		queue* getChannelList();
		int getNumOfBackoffs();
		int getNumOfFailures();//----------------
		int getNumOfInterference();//----------

		static void* masterThread(void* data);
		static void* thread_maker(void* data);
		void txData( );		
		void setcompletedTx() ;
		int getcompletedTx() ;
};

#endif	
