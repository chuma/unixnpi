/* UnixNPI 1.1.3 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "newtmnp.h"

#define TimeOut 30

/* Function prototype */
void SigAlrm(int sigNo);

int main(int argc, char *argv[])
{
	FILE *inFile;
	long inFileLen;
	long tmpFileLen;
	uchar sendBuf[MaxHeadLen + MaxInfoLen];
	uchar recvBuf[MaxHeadLen + MaxInfoLen];
	NewtDevInfo newtDevInfo;
	int newtFd;
	uchar ltSeqNo;
	int i, j, k;
	
	k = 1;
	
	/* Initialization */
	fprintf(stdout, "\n");
	fprintf(stdout, "UnixNPI - a Newton Package Installer for Unix platforms\n");
	fprintf(stdout, "Version 1.1.3 by Richard C. L. Li, Victor Rehorst, Chayam Kirshen\n");
	fprintf(stdout, "This program is distributed under the terms of the GNU GPL: see the file COPYING\n");

	/* Install time out function */
	if(signal(SIGALRM, SigAlrm) == SIG_ERR)
		ErrHandler("Error in setting up timeout function!!");

	/* Check arguments */
	if(argc < 2)
		ErrHandler("Usage: unixnpi [-s speed] PkgFiles...");
	else
	{
		if (strcmp(argv[1],"-s") == 0)
		{
			if (argc < 4)
				ErrHandler("Usage: unixnpi [-s speed] PkgFiles...");
			newtDevInfo.speed = atoi(argv[2]);
			k = 3;
		}
		else
		{
			newtDevInfo.speed = 38400;
			k = 1;
		}
	}

	strcpy(newtDevInfo.devName, "/dev/newton");

	/* Initialize Newton device */
	if((newtFd = InitNewtDev(&newtDevInfo)) < 0)
		ErrHandler("Error in opening Newton device!!\nDo you have a symlink to /dev/newton?");
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
	fprintf(stdout, ".\n");
	fflush(stdout);

	/* batch install all of the files */
	for (k; k < argc; k++)
	{
		/* load the file */
		if((inFile = fopen(argv[k], "rb")) == NULL)
			ErrHandler("Error in opening package file!!");
		fseek(inFile, 0, SEEK_END);
		inFileLen = ftell(inFile);
		rewind(inFile);
		printf("File is '%s'\n", argv[k]);

		/* Send LT frame newtdocklpkg */
		alarm(TimeOut);
		strcpy(sendBuf, "newtdocklpkg");
		tmpFileLen = inFileLen;
		for(i = 15; i >= 12; i--) {
			sendBuf[i] = tmpFileLen % 256;
			tmpFileLen /= 256;
			}
		do {
			SendLTFrame(newtFd, ltSeqNo, sendBuf, 16);
		} while(WaitLAFrame(newtFd, ltSeqNo) < 0);
		ltSeqNo++;
		/* fprintf(stdout, ".\n"); */

		fprintf(stdout, "Sending %d / %d\r", 0, inFileLen);
		fflush(stdout);
	
		/* Send package data */
		while(!feof(inFile)) {
			alarm(TimeOut);
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
		}
		fprintf(stdout, "Sending %d / %d\n", inFileLen, inFileLen);
	
		/* Wait LT frame newtdockdres */
		alarm(TimeOut);
		while(RecvFrame(newtFd, recvBuf) < 0 || recvBuf[1] != '\x04');
		SendLAFrame(newtFd, recvBuf[2]);

		fclose (inFile);

	} /* END OF FOR LOOP */

	/* Send LT frame newtdockdisc */
	alarm(TimeOut);
	do {
		SendLTFrame(newtFd, ltSeqNo, "newtdockdisc\0\0\0\0", 16);
	} while(WaitLAFrame(newtFd, ltSeqNo) < 0);
	
	/* Wait disconnect */
	alarm(0);
	WaitLDFrame(newtFd);
	fprintf(stdout, "Finished!!\n\n");
			
	/* fclose(inFile); */
	close(newtFd);
	return 0;
}

void SigAlrm(int sigNo)
{
	ErrHandler("Timeout error, connection stopped!!");
}
