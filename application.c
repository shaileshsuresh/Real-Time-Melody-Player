 	
#include "TinyTimber.h"
#include "sciTinyTimber.h"
#include "canTinyTimber.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* This code generates a 1KHz frequency tone.
 
 * You can start hearing the tone on pressing the key 'p' and increasing the volume using 'i'. Use 'd' to decrease the volume.
 * Press 'm' to mute the tone and press 'u' to unmute, which will play the tone in the previous volume.
 * Press 'q' and 'e' to increase and decrease the background load range respectively by 500.
 * Press 'v' and 'w' to enable and disable the deadline of the background load .
 * 
 * */



int *address = (int*) 0x4000741C;

/* App header */

typedef struct {
  Object super;
   Timer s;
   char buffer[10];
int timeperiod[25]; //array of time period
  int john[32];		//index 022444
  int tp_melody[32];		//time period_array for the key
  int beat_arr[32];		//len of beat
  bool isPlayin;
  bool isFPlay;// variable to terminate the freq_cntrl method
  //int tempo[32];
  bool enableprint;
  bool isplay;
  bool conductorMode;
  //bool isWatchDog;
  //bool enableprint;
  bool enablemute_m;
  bool isM1_c;
  bool isM2_c;
  bool exi_c;//flag to check if ther is an existing conductor
  bool nrx;
  bool case_s;
  bool can_dc_m;
  int m1_addr;
  int m2_addr;
  int index;
  int freq;
  int c;
  int mute_m;
  int rcvd;
  int boards;
  int counter;
  int orgi_addr;
  int addr;
  int watchdog_c;
  int volume;
  int bpm;
  int key_m;
  int beat_len;
  int conductor_veno_mwonuse;

  //int boards;
  int key; //entered key
 // int counter;
  //int vol;
  //int temp_vol;
  int j;
  Time temp;
  //Timer p;
} App;

#define initApp()                                                              \
  { initObject(), initTimer(), {0},{2024,1911,1803,1702,1607,1516,1431,1351,1275,1203,1136,1072,1012,955,901,851,803,758,715,675,637,601,568,536,506}, {0,2,4,0,0,2,4,0,4,5,7,4,5,7,7,9,7,5,4,0,7,9,7,5,4,0,0,-5,0,0,-5,0},{0},{500,500,500,500,500,500,500,500,500,500,1000,500,500,1000,250,250,250,250,500,500,250,250,250,250,500,500,500,500,1000,500,500,1000},true,false,false,true, false,false,false,false,false,false,true,true,0}

void reader(App *, int);
void receiver(App*, uchar);
void transmitter(App*, int);
void generate_ack_msg(App*, int);
/* Task header */

typedef struct {
  Object super;
  bool isRunning;
  bool isPPlay;
  bool isMute;
  bool enablemute;
  int vol;
  int temp_vol;
  int freq;
  Time temp;
} Task;

#define initTask()                                                             \
  { initObject(), true ,false, false,false, 10, 0}

void task1method(Task *, int);
void task2method(Task *, int);
void setIsRunning(Task *, bool);

/* Music player */

/*typedef struct {
  Object super;
  int timeperiod[25]; //array of time period
  int john[32];		//index 022444
  int tp_melody[32];		//time period_array for the key
  int beat_arr[32];		//len of beat
  bool isPlayin;
  bool isFPlay;// variable to terminate the freq_cntrl method
  //int tempo[32];
  bool enableprint;
  //int boards;
  int key; //entered key
  int counter;
  //int vol;
  //int temp_vol;
  int j;
  Time temp;
} Music;

#define initMusic()                                                             \
  { initObject(), {2024,1911,1803,1702,1607,1516,1431,1351,1275,1203,1136,1072,1012,955,901,851,803,758,715,675,637,601,568,536,506}, {0,2,4,0,0,2,4,0,4,5,7,4,5,7,7,9,7,5,4,0,7,9,7,5,4,0,0,-5,0,0,-5,0},{0},{500,500,500,500,500,500,500,500,500,500,1000,500,500,1000,250,250,250,250,500,500,250,250,250,250,500,500,500,500,1000,500,500,1000},0 }
*/
void play_tone(Task *, int);

void stopplaytone(Task *, char);
void stopfreq(App *, char);
void vol_to_app(App *, int);
void count_boards(App *, int);
void board_selection(App *, int);
void set_address(App *, int);
void broadcast_msg_c(App *, int);

/* Objects */
App app = initApp();
Serial sci0 = initSerial(SCI_PORT0, &app, reader);

Can can0 = initCan(CAN_PORT0, &app, receiver);
//Can can1 = initCan(CAN_PORT0, &app, transmitter);

Task task1 = initTask();
Task task2 = initTask();
//Music music1 = initMusic();

void update_vol(Task *self, char v){//increment/decrement the volume the limit is from 0-10
	switch(v){
		case 'i':
			if (self->vol<10){
			self->vol=self->vol+1;
			}
			break;
		case 'd':
			if (self->vol>0){
				self->vol=self->vol-1;
			}
			break;
	}
		//char buf[50];
		//sprintf(buf, " Vol is %d  , ", self->vol);
		//SCI_WRITE(&sci0, buf);
	
	ASYNC(&app, vol_to_app, self->vol);
	
	}
	
void set_vol_rec(Task * self, int v){
	self->vol = v;
}
void sent_mute(App *self, bool ism){
	if(ism){
		self->mute_m =1;
		}
		else{
			self->mute_m = 0;
			}
		//char buf[50];
		//sprintf(buf, " Vol is %d  , ", self->mute_m);
		//SCI_WRITE(&sci0, buf);
	}
	
void control_mute(Task *self, char m){
		//self->vol=0;
		self->isMute= !(self->isMute);
		if(self->isMute){
			self->temp_vol=self->vol;
			//self->vol = 0;
		}
		else{
			self->vol = self->temp_vol;
		}
		ASYNC(&app, sent_mute, self->isMute);
		//case 'u':
		//	self->vol=self->temp_vol;
		//	self->isMute=false;
		//	break;
		}
	
void control_mute_m(Task *self, int v){
	if(v==1){
		self->isMute = true;
		}
		else{
			self->isMute = false;
			}
		//char buf[50];
		//sprintf(buf, " mute is %d  , ", self->isMute);
		//SCI_WRITE(&sci0, buf);
			if(self->isMute){
			self->temp_vol=self->vol;
			//self->vol = 0;
		}
	}
	
	
void print_mute(Task *self, int unused){	
		if(self->enablemute){
			if(self->isMute){
			SCI_WRITE(&sci0, "MUTED\n");
			}
			else{
				SCI_WRITE(&sci0, "NOT MUTED\n");
				}
		}
		SEND(SEC(2), SEC(2), self, print_mute, 0);
				
}

// Method Music player
void tempo (App *self, int value){
	//if (value == 997){}
	if(value>240){
		value = 240;
		}
	else if(value<60){
		value = 60;
		} 
	int beat[32] = {500,500,500,500,500,500,500,500,500,500,1000,500,500,1000,250,250,250,250,500,500,250,250,250,250,500,500,500,500,1000,500,500,1000};
	for(int i=0;i<32;i++){
	self->beat_arr[i] = (120*beat[i])/value;
	
	}
}

void print_tempo_method(App *self, int unused){ // method to print tempo 5 sec
	
	//Code to print tempo every 2 seconds
	char buf[50];
	//self->temp = CURRENT_OFFSET();
		if(self->enableprint == true){	 
			sprintf(buf, " tempo is %d  , ", self->bpm);
			SCI_WRITE(&sci0, buf);
		}
	SEND(SEC(5), SEC(5), self, print_tempo_method, 0);
	}

//function to shift the melody depending on the key
void freq_calc(App *self, int value){    //calculating the key
	if(value>5){
		value = 5;
		}
	else if(value<-5){
		value = -5;
		} 
	for (int j=0; j<32; j++)
		{
			self->key = self->john[j]+value+10;
			self->tp_melody[j] = self->timeperiod[self->key];
			//char buf[50];
			//sprintf(buf, " tp_%d  , ", self->tp_melody[j]);
			//SCI_WRITE(&sci0, buf);
			//SYNC(&task1, play_tone, 0);
		}
}
//Shailesh
/*void watchdog_timer(App *self,int unused){
	// To calculate the number of boards --aravind 
	if(self->isM1_c && self->isM2_c){
		self->boards = self->boards + 2;
	}
	else if((self->isM1_c  && !(self->isM2_c)) || (self->isM2_c && !(self->isM1_c))){
		self->boards = self->boards + 1;
	}
	
	else{
		self->boards = self->boards - 1;
	}
}*/

//void calc_boards(Music *self, int boards){
//	self->boards = boards;
//}

void freq_asgn(Task *self, int value){
	self->freq = value;
	}

void freq_cntrl(App *self, int indi ){// almost done
	//if(self->isFPlay){//to terminate the method
		ASYNC(&task1, freq_asgn, self->tp_melody[indi]);
		char buf[50];
		sprintf(buf, "index is %d  , \n", indi);
		SCI_WRITE(&sci0, buf);
		AFTER(MSEC(self->beat_arr[indi]*0.8), &task1, setIsRunning , false);
		if(self->conductorMode){
		AFTER(MSEC(self->beat_arr[indi]), &task1, setIsRunning, true);
		}
		if(self->isPlayin){
			ASYNC(&task1, play_tone, 0);
				
			    //SCI_WRITE(&sci0, "ConD\n");
		
				self->isPlayin = false;
			}
		//SEND(MSEC(self->beat_arr[self->counter]), MSEC(self->beat_arr[self->counter]), self, freq_cntrl, 0);
		//self->counter = self->counter + 1;
	//}
	//else{
	//	return;
	//	}
	}
	
	
void freq_cntrl_m(App *self, int indi ){// almost done
	//if(self->isFPlay){//to terminate the method
		ASYNC(&task1, freq_asgn, self->tp_melody[indi]);
		char buf[50];
		sprintf(buf, "index is %d  , \n", indi);
		SCI_WRITE(&sci0, buf);
		AFTER(MSEC(self->beat_arr[indi]*0.8), &task1, stopplaytone , 's');
		AFTER(MSEC(self->beat_arr[indi]), &task1, stopplaytone, 'p');
		//if(self->conductorMode){
		//AFTER(MSEC(self->beat_arr[indi]), &task1, setIsRunning, true);
		//}
		//if(self->isPlayin){
			//SCI_WRITE(&sci0, "Muzic\n");
			ASYNC(&task1, play_tone, 0);
		//		self->isPlayin = false;
		//	}
		//SEND(MSEC(self->beat_arr[self->counter]), MSEC(self->beat_arr[self->counter]), self, freq_cntrl, 0);
		//self->counter = self->counter + 1;
	//}
	//else{
	//	return;
	//	}
}

void setIsRunning(Task *self, bool isRunning) { self->isRunning = isRunning; }

void play_tone(Task *self, int value) { //tone generator
			//char buf[50];
			//sprintf(buf, "PPLay: %d  , ", self->isPPlay);
			//SCI_WRITE(&sci0, buf);

	if (self->isPPlay == true){
		//char buf[50];
		//sprintf(buf, "Running:  %d  , ", self->isRunning);
		//SCI_WRITE(&sci0, buf);
	if(self->isRunning == false){
	SEND(USEC(self->freq), USEC(self->freq), self, play_tone, 0);
	}
		else{	if (value == 0){
				*address = self->vol;
				}
			else{
				*address = 0;
				
				}
		//char buf[50];
		//sprintf(buf, " %d  , ", self->isRunning);
		//SCI_WRITE(&sci0, buf);
			if(self->isMute == true){
				//char buf[50];
				//sprintf(buf, " %d  , ", self->isMute);
				//SCI_WRITE(&sci0, "Kunna thayalan\n");
				SEND(USEC(self->freq), USEC(self->freq), self, play_tone, 0);
				}
			else{
				//SCI_WRITE(&sci0, "Achan indo\n");
			SEND(USEC(self->freq), USEC(self->freq), self, play_tone, *address);
			
			}
			}
	}
	else{return;}
		//AFTER(USEC((self->beat[self->j])), self, play_tone, 0);

}

void control_play(App *self, int unused){ //v
	self->isPlayin = true;
	
	}
	
void grantin_c(App *self, int unused){
	CANMsg msg;
	SCI_WRITE(&sci0, "conductorship koduthu");
	msg.length = (uchar)6;
	msg.msgId = self->addr;
	msg.nodeId = self->conductor_veno_mwonuse;
	msg.buff[0] = (uchar)3;
	msg.buff[1] = (uchar)self->counter;
	msg.buff[2] = (uchar)self->bpm;
	msg.buff[3] = (uchar)self->key_m;
	msg.buff[4] = (uchar)self->mute_m;
	msg.buff[5]	= (uchar)self->volume;
	CAN_SEND(&can0, &msg);
	//ASYNC(&app, stopfreq, 's');
	//self->addr = self->orgi_addr;
	//ASYNC(&app, stopplaytone, 's');
	//self->conductorMode = false;
	//self->addr = self->orgi_addr;
	}
	
void empty_msg_fn(App *self, int unused){
	CANMsg msg;
	msg.length = (uchar)1;
	msg.buff[0]=(uchar)9;
	if(CAN_SEND(&can0, &msg)){
		SCI_WRITE(&sci0,"silent failure"); 
	}
	SEND(SEC(1),SEC(1), self, empty_msg_fn, 0);
	}	
	
void receiver(App *self, uchar unused) { //can receiver method //need to update
    CANMsg msg;
	//int key;
    CAN_RECEIVE(&can0, &msg);
    //SCI_WRITE(&sci0, "Can msg received:");
	if(self->nrx){
		SCI_WRITE(&sci0, "Nammal onnumilley\n");
		}
	else{
	if(self->conductorMode == true){
	//char buf[50];
	//sprintf(buf, " buff0 is %d , %d msg , ", (int)msg.buff[0],msg.msgId);
	//SCI_WRITE(&sci0, buf);
		if(msg.buff[0]==0 && msg.nodeId ==0){
			
			if(self->m1_addr==0){ 
				self->m1_addr = msg.msgId;
				self->isM1_c = true;
			}
			else if(self->m1_addr == msg.msgId){
				self->isM1_c = true;
			}
			else if(self->m2_addr == 0){
				self->m2_addr = msg.msgId;
				self->isM2_c = true;
				}
			else if(self->m2_addr ==msg.msgId){
				self->isM2_c = true;
			}
			if(self->m1_addr>self->m2_addr){
				int temp = self->m1_addr;
				self->m1_addr = self->m2_addr;
				self->m2_addr = temp;
				}
			//char bufs[50];
			//sprintf(bufs, "  m1nm2is %d \n , ", self->isM1_c);
			//
			//SCI_WRITE(&sci0, );
			//	else if(mm2_addr !=msg.msgId)
			//if(m1_addr!=msg.msgId & m2_addr!=msg.msgId){
			ASYNC(&app, count_boards, 0);
		}
		else if(msg.buff[0]==3 && msg.nodeId == 0){
				self->conductor_veno_mwonuse = msg.msgId;
				char buf[50];
				sprintf(buf, " new_conductor is %d  , ", self->conductor_veno_mwonuse);
				SCI_WRITE(&sci0, buf);
				ASYNC(&app, stopfreq, 's');
				//ASYNC(&app, stopplaytone, 's');
				self->conductorMode = false;
				self->addr = self->orgi_addr;
				//AFTER(MSEC(500), &app, grantin_c, 0);
				ASYNC( &app, grantin_c, 0);
				}
		//	self->watchdog_c = self->watchdog_c + 1;
	}
	else{//Musician // need to change the case statements
		if(msg.nodeId == 15 && msg.msgId==0){ //may be need to create another method
				//char buf[50];
				//sprintf(buf, " orgi_addr is %d  , ", self->orgi_addr);
			//	SCI_WRITE(&sci0, buf);
				if(self->can_dc_m){
					ASYNC(&app, empty_msg_fn, 0);
					self->can_dc_m = false;
					}
				self->exi_c = true;
			/*if(self->orgi_addr ==1 ){
				//AFTER(MSEC(self->beat_arr[self->counter]*0.94),&app, generate_ack_msg, 0);
				ASYNC(&app, generate_ack_msg, 0);
						}
			else if(self->orgi_addr ==2 ){
				//for(int i=0; i<1000*self->addr; i++){}
				AFTER(MSEC(10),&app, generate_ack_msg, 0);
				//ASYNC(&app, generate_ack_msg, 0);
				}	
			else if(self->orgi_addr ==3 ){
				//	for(int i=0; i<100000; i++){}
					AFTER(MSEC(20),&app, generate_ack_msg, 0);
					//ASYNC(&app, generate_ack_msg, 0);
				}*/
			AFTER(MSEC(1)*self->orgi_addr,&app, generate_ack_msg, 0);	
				//ASYNC(&app, generate_ack_msg, 0);
		}
		else if(msg.nodeId == self->addr){//checking if the msg is for this boards
			if(msg.buff[0] ==1){//code to check the action
				int key_tm;
				if(msg.buff[3] >128){
				key_tm = (int)msg.buff[3] - 256;}
				else{
					key_tm = (int)msg.buff[3];
					}
				//char buf[50];
				//sprintf(buf, "Key: %d |%d Vol: %d Tempo: %d  , ", msg.buff[3],key_tm, msg.buff[5], msg.buff[2]);
				//SCI_WRITE(&sci0, buf);
				//SCI_WRITE(&sci0, "playing tone\n");
				//ASYNC(&app, stopfreq, 'p');
				ASYNC(&task1, setIsRunning , true);
				//ASYNC(&app, control_play, 0);
				ASYNC(&app, freq_calc, 0);
				ASYNC(&app, tempo, (int)msg.buff[2]);
				ASYNC(&app, freq_calc,key_tm);
				ASYNC(&task1, set_vol_rec, (int)msg.buff[5]);
				if(self->enablemute_m){
					ASYNC(&task1, control_mute_m, 1);
				}
				else{
					ASYNC(&task1, control_mute_m, (int)msg.buff[4]);
				}
				ASYNC(&task1, stopplaytone, 'p');
				ASYNC(&app,freq_cntrl_m, (int)msg.buff[1]);
			}
			else if(msg.buff[0] ==2){
				self->case_s = false;
				}
			else if(msg.buff[0] == 3){
				Time fear_ayo_nee = MSEC(500);
				SCI_WRITE(&sci0, "kitty conductorship\n");
				self->conductorMode = true;
				self->exi_c = false;
				self->key = msg.buff[3];
				self->bpm = msg.buff[2];
				self->volume = msg.buff[5];
				self->counter = msg.buff[1];
				char buf[50];
				sprintf(buf, "index: %d , ", msg.buff[1]);
				SCI_WRITE(&sci0, buf);
				self->isplay = true;
				ASYNC(&task1, control_mute_m, (int)msg.buff[4]);
				ASYNC(&app, set_address, 0);
				ASYNC(&app, broadcast_msg_c, 0);
				ASYNC(&task1, stopplaytone, 'p');
				ASYNC(&app, stopfreq, 'p');
				self->boards = 1;
				//if(self->isplay){
					SCI_WRITE(&sci0, "inside isplay\n");
					//ASYNC(&app, control_play, 0);
					ASYNC(&app, freq_calc, 0);
					ASYNC(&app, stopfreq, 'p');
					if(self->case_s){
						AFTER(fear_ayo_nee, &app, board_selection, 0);
						self->isplay = false;
					}
					else{	
						self->isplay = true;
						}
					
				}
		}	
		}
	//SCI_WRITE(&sci0, "\n");
	}
}

void calc_len_beat(App *self, int m){
	self->beat_len = m;
	}
	
/*void beat_arry_to_app(App *self, int c){
	//int value;
	ASYNC(&app, calc_len_beat, self->beat_arr[c]);
	//char buf[50];
	//sprintf(buf, " b is %d ** %d  , ", value, c);
	//SCI_WRITE(&sci0, buf);
	//return value;
	}*/

	
void count_boards(App *self, int unused){
		if(self->isM1_c && self->isM2_c){
		self->boards = self->boards + 1;
	}
	else if((self->isM1_c  && !(self->isM2_c)) || (self->isM2_c && !(self->isM1_c))){
		self->boards = self->boards + 1;
	}
	
	else{
		//self->boards = self->boards - 1;
	}
		char buf[50];
		sprintf(buf, " boards is %d  , ", self->boards);
		SCI_WRITE(&sci0, buf);
}

void vol_to_app(App *self, int vol){
	self->volume = vol;
}

void board_selection(App *self, int value){ //controls the melody playin//not completed
	CANMsg msg;
	msg.length = (uchar)6;
	//char bufs[50];
	//sprintf(bufs, "  kappy %d \n , ", self->boards);
	//SCI_WRITE(&sci0, bufs);
	//ASYNC(&app, beat_arry_to_app, self->counter);
	if(self->conductorMode){
		if(self->isFPlay){
			if(self->counter<32){
				AFTER(MSEC(self->beat_arr[self->counter]*0.8),&app, broadcast_msg_c, 0);
			if(self->boards==3){
				if(self->counter % self->boards == 0){ //tone gen for conductor
					ASYNC(&task1, setIsRunning, true);
					self->watchdog_c = 0; //resetting the counter for the number of boards as 0 n we are counting it based on the recieved ack msg
					ASYNC(&app,freq_cntrl,self->counter);
				}
				else if(self->counter % self->boards == 1){	//tone gen for musician board 1
						//ASYNC(&app, broadcast_msg_c, 0);
						ASYNC(&task1, setIsRunning, false);
						msg.msgId = (uchar)self->addr;
						msg.nodeId = (uchar)self->m1_addr;
						/*if(self->case_s){
							msg.buff[0] = (uchar)2;
							}
						else{	
							msg.buff[0] = (uchar)1;
						}*/
						msg.buff[0] = (uchar)1;
						msg.buff[1] = (uchar)self->counter;
						msg.buff[2] = (uchar)self->bpm;
						msg.buff[3] = (unsigned char)self->key_m;
						msg.buff[4] = (uchar)self->mute_m;
						msg.buff[5]	= (uchar)self->volume;
						if(CAN_SEND(&can0, &msg)){
							ASYNC(&task1, setIsRunning, true);
							ASYNC(&app,freq_cntrl,self->counter);
						}
						
					} 
				else if(self->counter % self->boards ==2){ //tone gen for musician board 2
						//ASYNC(&app, broadcast_msg_c, 0);
						ASYNC(&task1, setIsRunning, false);
						msg.msgId = (uchar)self->addr;
						msg.nodeId = (uchar)self->m2_addr;
						/*if(self->case_s){
							msg.buff[0] = (uchar)2;
							}
						else{	
							msg.buff[0] = (uchar)1;
						}*/
						msg.buff[0] = (uchar)1;
						msg.buff[1] = (uchar)self->counter;
						msg.buff[2] = (uchar)self->bpm;
						msg.buff[3] = (unsigned char)self->key_m;
						msg.buff[4] = (uchar)self->mute_m;
						msg.buff[5]	= (uchar)self->volume;
						if(CAN_SEND(&can0, &msg)){
							ASYNC(&task1, setIsRunning, true);
							ASYNC(&app,freq_cntrl,self->counter);
						}
				}
			}
			else if (self->boards ==2){
				if(self->counter % self->boards == 0){ //tone gen for conductor
					//self->watchdog_c = 0;
					ASYNC(&task1, setIsRunning, true);
					ASYNC(&app,freq_cntrl,self->counter);
					
				}
				else if(self->counter % self->boards == 1){	//tone gen for musician board 1
					//ASYNC(&app, broadcast_msg_c, 0);
					self->isM1_c = false;
					ASYNC(&task1, setIsRunning, false);
					msg.msgId = (uchar)0;
					msg.nodeId = (uchar)self->m1_addr;
					/*if(self->case_s){
							msg.buff[0] = (uchar)2;
							}
						else{	
							msg.buff[0] = (uchar)1;
						}*/
					msg.buff[0] = (uchar)1;
					msg.buff[1] = (uchar)self->counter;
					msg.buff[2] = (uchar)self->bpm;
					msg.buff[3] = (unsigned char)self->key_m;
					msg.buff[4] = (uchar)self->mute_m;
					msg.buff[5]	= (uchar)self->volume;
					if(CAN_SEND(&can0, &msg)){
						ASYNC(&task1, setIsRunning, true);
						ASYNC(&app,freq_cntrl,self->counter);
						}
				}
			}	
			else if(self->boards==1){
				//ASYNC(&app, broadcast_msg_c, 0);
				ASYNC(&task1, setIsRunning, true);
				ASYNC(&app,freq_cntrl,self->counter);//tone gen for conductor board
				
				//char buf[50];
				//sprintf(buf, " c is %d mmm  , ", self->counter);
				//SCI_WRITE(&sci0, buf);
			}
			//AFTER(MSEC(self->beat_arr[self->counter]*0.94), &task1, setIsRunning , false);
			//AFTER(MSEC(self->beat_arr[self->counter]), &task1, setIsRunning, true);
			//CAN_SEND(&can0, &msg);
			//char buf[50];
			//sprintf(buf, " index is %d  , ", self->counter);
			//SCI_WRITE(&sci0, buf);
			SEND(MSEC(self->beat_arr[self->counter]), MSEC(self->beat_arr[self->counter]), self, board_selection, 0);
			self->counter=self->counter+1;
				}
			else{
				SEND(MSEC(1), MSEC(1), self, board_selection, 0);
				self->counter=0;
				}
			//self->beat_len = (int)ASYNC(&app, beat_arry_to_app, self->counter);
			//SEND(MSEC(self->beat_arr[self->counter]), MSEC(self->beat_arr[self->counter]), self, board_selection, 0);
		}
		else{
			ASYNC(&app, stopplaytone, 's');
			return;
			}
	}
	///else{
		//	ASYNC(&app, stopplaytone, 's');
		//}
}


void generate_ack_msg(App *self, int unused){//generate the ack msg by the musicain for the broadcast msg
	CANMsg msg;
	msg.length = (uchar)1;
	msg.nodeId = (uchar)0;//unicast msg for conductor 
	msg.msgId = (uchar)self->addr;//set the address of the musician
	msg.buff[0] = (uchar)0; //return action for the broadcast msg
	//char buf[50];
	//sprintf(buf, " ack is %d  , ", msg.buff[0]);
	//SCI_WRITE(&sci0, buf);
	SCI_WRITE(&sci0, "ack generated!\n");
	CAN_SEND(&can0, &msg);
} 

void broadcast_msg_c(App *self, int unused){
	CANMsg msg;
	msg.length = (uchar)1;
	if(self->conductorMode){
		//for broadcast messge to calculate number of boards
		//self->isM1_c = false;
		//self->isM2_c = false;
		self->boards = 1;
		msg.msgId = (uchar)0;
		msg.nodeId = (uchar)15;
		msg.buff[0]=(uchar)0;//broadcast
		char buf[50];
		sprintf(buf, " broadcast  %d, ",msg.buff[0]);
		SCI_WRITE(&sci0, buf);
		CAN_SEND(&can0, &msg);
	}
}

void stopfreq(App *self, char c){
	if(c=='s')
	{
		self->isFPlay = false;
		//self->counter = 0;
		}
	else{
		self->isFPlay = true;
		self->isPlayin = true;
		}
//		char buf[50];
//		sprintf(buf, "stop freq %d  , ", self->isFPlay);
//		SCI_WRITE(&sci0, buf);
}
		
void stopplaytone(Task *self, char c){
	if(c=='p')
	{
		self->isPPlay = true;
		}
	else{
		self->isPPlay = false;
		}
	char buf[50];
	sprintf(buf, " stop play %d  , ", self->isPPlay);
	SCI_WRITE(&sci0, buf);
}


void set_address(App *self, int unused){//method to set address for the muscian and conductor
	if(self->conductorMode){
		self->addr = 0;
	}
	else{
		self->addr = self->orgi_addr;
	}
	//char buf[50];
	//sprintf(buf, " addr is %d  , ", self->addr);
	//SCI_WRITE(&sci0, buf);
	}

void requi_c(App *self, int unused){
	CANMsg msg;
	msg.length = 1;
	msg.msgId = self->addr;
	msg.nodeId = 0;
	msg.buff[0] = 3;
	SCI_WRITE(&sci0, "conductorship THAAADA Meire");
	CAN_SEND(&can0, &msg);
	}
	
void nrx_fn(App *self, int unused){
	self->nrx = false;
	SCI_WRITE(&sci0, "Nthada mwonu nee fear ayo \n");
	}
	
void enable_mute_m(Task *self, int unused){
	self->enablemute = !(self->enablemute);
}

void print_casem(App *self, int unused){
	if(self->boards ==1){
		char buf[50];
		sprintf(buf, " addr is %d  , ", self->addr);
		SCI_WRITE(&sci0, "Only conductor is active\n");
		}
	else if(self->boards == 2){
		char buf[50];
		sprintf(buf, " Musician %d is connected , \n", self->m1_addr);
		SCI_WRITE(&sci0, buf);
		}
	else if(self->boards == 3){
		char buf[50];
		sprintf(buf, " Musician %d and %d  , \n", self->m1_addr, self->m2_addr);
		SCI_WRITE(&sci0, buf);
		}
	}

void reader(App *self, int c) {
	Time Diff = T_SAMPLE(&self->s);
	//msg.length = (uchar)5;
  if (c == '\n') {
    return;
  }
  SCI_WRITE(&sci0, "Rcv: \'");
  SCI_WRITECHAR(&sci0, c);
  SCI_WRITE(&sci0, "\'\n");
  CANMsg msg;
	char buuf[50];
	  switch(c) {
		  // select conductor/ musician
		  case 'c':
			//need to assign conductorship according to prio
			if(self->exi_c){
				ASYNC(&app, requi_c, 0);
				}
			else{
			self->conductorMode = true;
			ASYNC(&app, set_address, 0);
			ASYNC(&app, broadcast_msg_c, 0);
			self->boards = 1;
			}
			//char buf[50];
			//sprintf(buf, "b is n %d  , ", self->boards);
			//SCI_WRITE(&sci0, buf);
			break;
			case 'b':
				self->conductorMode = false;
			break;
	  	case '0'...'9':   //case to read the numbers
		case '-':
			self->buffer[self->index++] = c;
			break;
		case 'i':
		case 'd':
			if(self->conductorMode == true){
				SYNC(&task1, update_vol, c);
			}
			break;
		case 'k':
			self->buffer[self->index] = '\0';
			int value= atoi(self->buffer);
			self->key_m = value;
			if(self->key_m>10){
				self->key_m = 5;
				}
			if(self->conductorMode == true){
				ASYNC(&app, freq_calc, value);
			}
			self->index=0;
			break;
		case 'y':
				self->buffer[self->index] = '\0';
				 value= atoi(self->buffer);
				 self->bpm = value;
				 if(self->bpm > 240){
					 self->bpm = 240;
					 }
				else if(self->bpm <60){
					self->bpm = 60;
					}
				if(self->conductorMode == true){
				ASYNC(&app, tempo, value);
				}
				self->index=0;
				break;
		case 'm':
			ASYNC(&app, broadcast_msg_c, 0);
			AFTER(MSEC(500),&app, print_casem, 0);
			break;
		case 't':
			if(self->conductorMode == true){
				ASYNC(&task1, control_mute, c);
				//ASYNC(&task1, print_mute, 0);
			}
			else{
				self->enablemute_m = !(self->enablemute_m);
				}
			break;
		case 'p':
			//self->watchdog_c = 0;
			//ASYNC(&app, stopfreq, c);
			sprintf(buuf, "No of boards is n %d  , ", self->boards);
			SCI_WRITE(&sci0, buuf);
			if(Diff > MSEC(300)){
			ASYNC(&task1, stopplaytone, c);
			//self->isplay = true;
			self->case_s = true;
			ASYNC(&app, stopfreq, c);
			if(self->conductorMode == true){
			if(self->isplay){
				//ASYNC(&app, control_play, 0);
				ASYNC(&app, freq_calc, 0);
				ASYNC (&app, board_selection, 0);
				self->isplay = false;
			}
			}
			}
			break;
		case 's':
			//starts = CURRENT_OFFSET();
			self->isplay = true;
			//self->case_s = true;
			if(self->conductorMode== true){
				self->counter = 0;
				ASYNC(&task1, stopplaytone, c);
				ASYNC(&app, stopfreq, c);
				msg.length = (uchar)1;
				msg.buff[0] = (uchar)2;
				msg.nodeId = (uchar)self->m1_addr;
				msg.msgId = (uchar)self->addr;
				CAN_SEND(&can0, &msg);
				msg.nodeId = (uchar)self->m2_addr;
				CAN_SEND(&can0, &msg);
			}
			/*else{
			msg.msgId = (uchar)6;
			msg.nodeId =(uchar)3;
			msg.buff[0] = (uchar)c;
			}
			CAN_SEND(&can0, &msg);*/
			T_RESET(&self->s);
			break;
			
		case 'r':
			if(self->conductorMode ==true){
				ASYNC(&app, freq_calc, 0);
				ASYNC(&app, tempo, 120);
				self->key_m = 0;
				self->bpm = 120;
			}
			break;
			
		//to enable printing tempo	
		case 'e':
			if(self->conductorMode){
				self->enableprint = !(self->enableprint);
			ASYNC(&app, print_tempo_method, self->enableprint);
			}
			else{
				ASYNC(&task1, enable_mute_m, 0);
				ASYNC(&task1, print_mute, 0);
				}
			break;
			case 'a':
				self->buffer[self->index] = '\0';
				 value= atoi(self->buffer);
				 self->addr = value;
				 self->orgi_addr = value;
				self->index=0;
				break;
			case 'z':
				self->nrx = !(self->nrx);
				break;
			case 'x':
				self->nrx = true;
				AFTER(SEC(3), &app, nrx_fn, 0);
				break;

	  }
	  //CAN_SEND(&can0, &msg);
  }

void initialize_b(App *self, int unused){
	self->conductorMode = false;
	self->orgi_addr = 2;
	self->addr = 2;
	self->key = 0;
	self->bpm = 120;
	self->volume = 10;
	self->boards = 1;
	//char buf[50];
	//sprintf(buf, " addr is %d, %d  , ",self->conductorMode, self->addr);
	//SCI_WRITE(&sci0, buf);
	}
	
void startApp(App *self, int unused) {
    CAN_INIT(&can0);
	SCI_INIT(&sci0);
	SCI_WRITE(&sci0, "Hello, Real-time Systems!\n");
	ASYNC(&app, initialize_b, 0);
}

/* main */

int main() {
  INSTALL(&sci0, sci_interrupt, SCI_IRQ0);
  INSTALL(&can0, can_interrupt, CAN_IRQ0);
  //INSTALL(&can1, can_interrupt, CAN_IRQ0);
  TINYTIMBER(&app, startApp, 0);
  return 0;
}

//Tasks to be done next
// Need to configure nodeID for broadcast and individual boards and to configure a device as conductor(MsgID = x00)
//MsgID should be configured as sender and nodeID as target
//Assign the frequency to different boards as required- This should depend on the number of boards
//

//Mute toggle - done
//switched M and T
//Print Mute and Tempo - done
//Conductor selection according to priority
