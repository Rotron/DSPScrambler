#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define N 129

#include "dspProcess.h"

const int hL = N;
const short h[N] = {
       16,    -20,     -4,      6,    -20,     11,      2,    -22,     26,
      -12,    -16,     36,    -33,      3,     35,    -53,     34,     15,
      -61,     69,    -25,    -45,     93,    -77,      0,     89,   -124,
       70,     45,   -143,    148,    -41,   -111,    202,   -153,    -18,
      200,   -257,    128,    116,   -307,    294,    -58,   -257,    425,
     -296,    -77,    451,   -544,    237,    306,   -716,    654,    -70,
     -701,   1109,   -742,   -339,   1521,  -1903,    799,   1808,  -5141,
     7927,  23757,   7927,  -5141,   1808,    799,  -1903,   1521,   -339,
     -742,   1109,   -701,    -70,    654,   -716,    306,    237,   -544,
      451,    -77,   -296,    425,   -257,    -58,    294,   -307,    116,
      128,   -257,    200,    -18,   -153,    202,   -111,    -41,    148,
     -143,     45,     70,   -124,     89,      0,    -77,     93,    -45,
      -25,     69,    -61,     15,     34,    -53,     35,      3,    -33,
       36,    -16,    -12,     26,    -22,      2,     11,    -20,      6,
       -4,    -20,     16
};

const int sineL = N;
const short sine[3]
 = {0,28377,-28377};

short fir(buffer *xn){
	int j;
	int yn = 0;
	int yn1 = 0;
	int conv1 = 0;
	int conv2 = 0;
	// performs the convolution of xn with B
	for(j=0; j < hL; j++){
		yn += readn(xn,j)*h[j];
	}
//    	for(j=0; j < sineL; j++){
//		conv2 += readn(xn,j)*sine[j];
//	}
//    	yn = (conv1 * conv2)/(2*N);
	yn = (yn >> 15) & 0xffff; //converts from Q30 to Q15 fixed point
	return (short)yn; // must cast to a 16-bit short for output to ALSA
}

// core dsp block processing
int dspBlockProcess(short *outputBuffer, short *inputBuffer, buffer *xnL, buffer *xnR, int samples, int * filter_on, double * volume){
	int i;
	short sine_val=0;
	int count = 0;
	int index = 0;
	int wn = 0;
	if(*filter_on == 0) {
		memcpy((char *)outputBuffer, (char *)inputBuffer, 2*samples); // passthru
	}
	else if(*filter_on == 1) {
		for (i=0; i < samples; i+=2){
			push(xnL,inputBuffer[i]); // stores the most recent sample in the circular buffer xn
			index = count % 3;
			sine_val = sine[index];
			count++;
			wn = (sine_val*fir(xnL));
			wn = (wn >> 15) & 0xffff;
			push(xnR,(short)wn);
			outputBuffer[i] = (short)(*volume*fir(xnR));  // filters the input and stores it in the left output channel
			outputBuffer[i+1] = 0; // zeros out the right output channel
		}	
	}
	return DSP_PROCESS_SUCCESS;
}
