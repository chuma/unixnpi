/* UnixNPI 1.1.1 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "newtmnp.h"

#define TimeOut 30

/* Function prototype */
void SigAlrm(int sigNo);

void main(int argc, char *argv[])
{
	FILE *inFile;
	long inFileLen;
	long tmpFileLen;
	uchar sendBuf[MaxHeadLen + MaxInfoLen];
	uchar recvBuf[MaxHeadLen + MaxInfoLen];
	char regCode[256];
	int regMode1, regMode2;
	NewtDevInfo newtDevInfo;
	int newtFd;
	uchar ltSeqNo;
	int i, j;
	
	/* Initialization */
	regCode[0] = '\0';
	fprintf(stdout, "\n");
	fprintf(stdout, "UnixNPI - a Newton Package Installer for Unix platforms\n");
	fprintf(stdout, "Version 1.1.1 by Richard C. L. Li\n");
	fprintf(stdout, "This program is distributed under the terms of
the GNU GPL: see the file COPYING\n");

	/* Install time out function */
	if(signal(SIGALRM, SigAlrm) == SIG_ERR)
		ErrHandler("Error in setting up timeout function!!");

	/* Get user configuration */
	strcpy(sendBuf, getenv("HOME"));
	strcat(sendBuf, "/.unixnpi.rc");
	if((inFile = fopen(sendBuf, "r")) == NULL)
		ErrHandler("Error in opening configuration file!!");
	fscanf(inFile, "NewtDev = %s\n", newtDevInfo.devName);
	fscanf(inFile, "Speed = %d\n", &newtDevInfo.speed);
	fscanf(inFile, "RegCode = %s\n", regCode);
	fclose(inFile);

	/* Open package file */
	if(argc != 2)
		ErrHandler("Usage: unixnpi <Package File Name>");
	if((inFile = fopen(argv[1], "rb")) == NULL)
		ErrHandler("Error in opening package file!!");
	fseek(inFile, 0, SEEK_END);
	inFileLen = ftell(inFile);
	rewind(inFile);
	
	/* Initialize Newton device */
	if((newtFd = InitNewtDev(&newtDevInfo)) < 0)
		ErrHandler("Error in opening Newton device!!");
	ltSeqNo = 0;
	
	/* Waiting to connect */
	fprintf(stdout, "\nWaiting to connect\n");
	do {
		while(RecvFrame(newtFd, recvBuf) < 0);
		} while(recvBuf[1] != '\x01');
	fprintf(stdout, "Connected\n");
	fprintf(stdout, "Handshaking");
	fflush(stdout);
	
	/* Send LR frame */
	alarm(TimeOut);
	do {
		SendFrame(newtFd, LRFrame, NULL, 0);
		} while(WaitLAFrame(newtFd, ltSeqNo) < 0);
	ltSeqNo++;
	fprintf(stdout, ".");
	fflush(stdout);
	
	/* Wait LT frame newtdockrtdk */
	while(RecvFrame(newtFd, recvBuf) < 0 || recvBuf[1] != '\x04');
	SendLAFrame(newtFd, recvBuf[2]);
	fprintf(stdout, ".");
	fflush(stdout);

	/* Send LT frame newtdockdock */
	alarm(TimeOut);
	do {
		SendLTFrame(newtFd, ltSeqNo, "newtdockdock\0\0\0\4\0\0\0\4", 20);
		} while(WaitLAFrame(newtFd, ltSeqNo) < 0);
	ltSeqNo++;
	fprintf(stdout, ".");
	fflush(stdout);
	
	/* Wait LT frame newtdockname */
	alarm(TimeOut);
	while(RecvFrame(newtFd, recvBuf) < 0 || recvBuf[1] != '\x04');
	SendLAFrame(newtFd, recvBuf[2]);
	fprintf(stdout, ".");
	fflush(stdout);
	
	/* Get owner name */
	i = recvBuf[19] * 256 * 256 * 256 + recvBuf[20] * 256 * 256 + recvBuf[21] *
		256 + recvBuf[22];
	i += 24;
	j = 0;
	while(recvBuf[i] != '\0') {
		sendBuf[j] = recvBuf[i];
		j++;
		i += 2;
		}
	sendBuf[j] = '\0';

	/* Check registration */
	if((i = strlen(sendBuf)) < 6) {
		j = 0;
		while(i < 6)
			sendBuf[i++] = sendBuf[j++] + 3;
		sendBuf[i] = '\0';
		}
	Encrypt(recvBuf, sendBuf);
	recvBuf[8] = '\0';
	regMode1 = strcmp(recvBuf, regCode);
	regMode2 = ~ strcmp(recvBuf, regCode);

	/* Send LT frame newtdockstim */
	alarm(TimeOut);
	do {
		SendLTFrame(newtFd, ltSeqNo, "newtdockstim\0\0\0\4\0\0\0\x1e", 20);
		} while(WaitLAFrame(newtFd, ltSeqNo) < 0);
	ltSeqNo++;
	fprintf(stdout, ".");
	fflush(stdout);

	/* Wait LT frame newtdockdres */
	alarm(TimeOut);
	while(RecvFrame(newtFd, recvBuf) < 0 || recvBuf[1] != '\x04');
	SendLAFrame(newtFd, recvBuf[2]);
	fprintf(stdout, ".");
	fflush(stdout);

	regMode1 = 0;
	regMode2 = 0;
	
	/* Send LT frame newtdocklpkg */
	alarm(TimeOut);
	strcpy(sendBuf, "newtdocklpkg");
	tmpFileLen = inFileLen;
	if(regMode1 != 0 && inFileLen * 2 + 3 > 32768 * 2 + 3)
		tmpFileLen = 32768 + 256;
	if(~regMode2 != 0 && inFileLen * 3 + 5 > 32768 * 3 + 5)
		tmpFileLen = 32768 + 256;
	for(i = 15; i >= 12; i--) {
		sendBuf[i] = tmpFileLen % 256;
		tmpFileLen /= 256;
		}
	do {
		SendLTFrame(newtFd, ltSeqNo, sendBuf, 16);
		} while(WaitLAFrame(newtFd, ltSeqNo) < 0);
	ltSeqNo++;
	fprintf(stdout, ".\n");

	/* Print unregister screen */
	if(regMode1 != 0) {
		fprintf(stdout, "\n");
		fprintf(stdout, "***************************************************\n");
		fprintf(stdout, "**               Unregistered Copy               **\n");
		fprintf(stdout, "**   Package size will be limited to 32K bytes   **\n");
		fprintf(stdout, "**                                               **\n");
		fprintf(stdout, "**   UnixNPI is a shareware, please register!!   **\n");
		fprintf(stdout, "***************************************************\n");
		fprintf(stdout, "\n");
		}

	fprintf(stdout, "Sending %d / %d\r", 0, inFileLen);
	fflush(stdout);
	
	/* Send package data */
	while(!feof(inFile)) {
		alarm(TimeOut);
		if(regMode1 != 0 && ftell(inFile) * 3 + 7 > 32768 * 3 + 7)
			SigInt(0);
		i = fread(sendBuf, sizeof(uchar), MaxInfoLen, inFile);
		while(i % 4 != 0)
			sendBuf[i++] = '\0';
		do {
			SendLTFrame(newtFd, ltSeqNo, sendBuf, i);
			} while(WaitLAFrame(newtFd, ltSeqNo) < 0);
		ltSeqNo++;
		if(ltSeqNo % 4 == 0) {
			fprintf(stdout, "Sending %d / %d\r", ftell(inFile), inFileLen);
			fflush(stdout);
			}
		if(~regMode2 != 0 && ftell(inFile) * 2 + 5 > 32768 * 2 + 5)
			SigInt(0);
		}
	fprintf(stdout, "Sending %d / %d\n", inFileLen, inFileLen);
	
	/* Wait LT frame newtdockdres */
	alarm(TimeOut);
	while(RecvFrame(newtFd, recvBuf) < 0 || recvBuf[1] != '\x04');
	SendLAFrame(newtFd, recvBuf[2]);
	
	/* Send LT frame newtdockdisc */
	alarm(TimeOut);
	do {
		SendLTFrame(newtFd, ltSeqNo, "newtdockdisc\0\0\0\0", 16);
		} while(WaitLAFrame(newtFd, ltSeqNo) < 0);
	
	/* Wait disconnect */
	alarm(0);
	WaitLDFrame(newtFd);
	fprintf(stdout, "Finished!!\n\n");
			
	fclose(inFile);
	close(newtFd);
	return 0;
}

void SigAlrm(int sigNo)
{
	ErrHandler("Timeout error, connection stopped!!");
}
