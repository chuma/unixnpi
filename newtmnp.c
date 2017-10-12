#include <stdio.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "newtmnp.h"

/* Constants definition */
uchar FrameStart[] = "\x16\x10\x02";
uchar FrameEnd[] = "\x10\x03";
uchar LRFrame[] = {
	'\x17', /* Length of header */
	'\x01', /* Type indication LR frame */
	'\x02', /* Constant parameter 1 */
	'\x01', '\x06', '\x01', '\x00', '\x00', '\x00', '\x00', '\xff',
		/* Constant parameter 2 */
	'\x02', '\x01', '\x02', /* Octet-oriented framing mode */
	'\x03', '\x01', '\x01', /* k = 1 */
	'\x04', '\x02', '\x40', '\x00', /* N401 = 64 */
	'\x08', '\x01', '\x03' /* N401 = 256 & fixed LT, LA frames */
	};
uchar LDFrame[] = {
	'\x04', /* Length of header */
	'\x02', /* Type indication LD frame */
	'\x01', '\x01', '\xff'
	};

int intNewtFd;

int InitNewtDev(NewtDevInfo *newtDevInfo)
{
	int newtFd;
	struct termios newtTty;

	/*  Install Ctrl-C function */
	intNewtFd = -1;
	if(signal(SIGINT, SigInt) == SIG_ERR)
		ErrHandler("Error in setting up interrupt function!!");
		
	/* Open the Newton device */
	if((newtFd = open(newtDevInfo->devName, O_RDWR)) == -1)
		return -1;
	
	/* Get the current device settings */
	tcgetattr(newtFd, &newtTty);
	
	/* Change the device settings */
	newtTty.c_iflag = IGNBRK | INPCK;
	newtTty.c_oflag = 0;
	newtTty.c_cflag = CREAD | CLOCAL | CS8 & ~PARENB & ~PARODD & ~CSTOPB;
	newtTty.c_lflag = 0;
	newtTty.c_cc[VMIN] = 1;
	newtTty.c_cc[VTIME] = 0;
	
	/* Select the communication speed */
	switch(newtDevInfo->speed) {
		#ifdef B2400
		case 2400:
			cfsetospeed(&newtTty, B2400);
			cfsetispeed(&newtTty, B2400);
			break;
		#endif
		#ifdef B4800
		case 4800:
			cfsetospeed(&newtTty, B4800);
			cfsetispeed(&newtTty, B4800);
			break;
		#endif
		#ifdef B9600
		case 9600:
			cfsetospeed(&newtTty, B9600);
			cfsetispeed(&newtTty, B9600);
			break;
		#endif
		#ifdef B19200
		case 19200:
			cfsetospeed(&newtTty, B19200);
			cfsetispeed(&newtTty, B19200);
			break;
		#endif
		#ifdef B38400
		case 38400:
			cfsetospeed(&newtTty, B38400);
			cfsetispeed(&newtTty, B38400);
			break;
		#endif
		#ifdef B57600
		case 57600:
			cfsetospeed(&newtTty, B57600);
			cfsetispeed(&newtTty, B57600);
			break;
		#endif
		#ifdef B115200
		case 115200:
			cfsetospeed(&newtTty, B115200);
			cfsetispeed(&newtTty, B115200);
			break;
		#endif
		#ifdef B230400
		case 230400:
			cfsetospeed(&newtTty, B230400);
			cfsetispeed(&newtTty, B230400);
			break;
		#endif
		default:
			close(newtFd);
			return -1;
		}
	
	/* Flush the device and restarts input and output */
	tcflush(newtFd, TCIOFLUSH);
	tcflow(newtFd, TCOON);
	
	/* Update the new device settings */
	tcsetattr(newtFd, TCSANOW, &newtTty);

	/* Return with file descriptor */
	intNewtFd = newtFd;
	return newtFd;
}

void FCSCalc(ushort *fcsWord, unsigned char octet)
{
	int i;
	uchar pow;
	
	pow = 1;
	for(i = 0; i < 8; i++) {
		if((((*fcsWord % 256) & 0x01) == 0x01) ^ ((octet & pow) == pow))
			*fcsWord = (*fcsWord / 2) ^ 0xa001;
		else
			*fcsWord /= 2;
		pow *= 2;
		}
}

void SendFrame(int newtFd, uchar *head, uchar *info, int infoLen)
{
	char errMesg[] = "Error in writing to Newton device, connection stopped!!";
	int i;
	ushort fcsWord;
	uchar buf;
	
	/* Initialize */
	fcsWord = 0;
	
	/* Send frame start */
	if(write(newtFd, FrameStart, 3) < 0)
		ErrHandler(errMesg);
	
	/* Send frame head */
	for(i = 0; i <= head[0]; i++) {
		FCSCalc(&fcsWord, head[i]);
		if(write(newtFd, &head[i], 1) < 0)
			ErrHandler(errMesg);
		if(head[i] == FrameEnd[0]) {
			if(write(newtFd, &head[i], 1) < 0)
				ErrHandler(errMesg);
			}
		}
	
	/* Send frame information */
	if(info != NULL) {
		for(i = 0; i < infoLen; i++) {
			FCSCalc(&fcsWord, info[i]);
			if(write(newtFd, &info[i], 1) < 0)
				ErrHandler(errMesg);
			if(info[i] == FrameEnd[0]) {
				if(write(newtFd, &info[i], 1) < 0)
					ErrHandler(errMesg);
				}
			}
		}

	/* Send frame end */
	if(write(newtFd, FrameEnd, 2) < 0)
		ErrHandler(errMesg);
	FCSCalc(&fcsWord, FrameEnd[1]);

	/* Send FCS */
	buf = fcsWord % 256;
	if(write(newtFd, &buf, 1) < 0)
		ErrHandler(errMesg);
	buf = fcsWord / 256;
	if(write(newtFd, &buf, 1) < 0)
		ErrHandler(errMesg);

	return;
}

void SendLTFrame(int newtFd, uchar seqNo, uchar *info, int infoLen)
{
	uchar ltFrameHead[3] = {
		'\x02', /* Length of header */
		'\x04', /* Type indication LT frame */
		};
	
	ltFrameHead[2] = seqNo;
	SendFrame(newtFd, ltFrameHead, info, infoLen);

	return;
}
	
void SendLAFrame(int newtFd, uchar seqNo)
{
	uchar laFrameHead[4] = {
		'\x03', /* Length of header */
		'\x05', /* Type indication LA frame */
		'\x00', /* Sequence number */
		'\x01' /* N(k) = 1 */
		};

	laFrameHead[2] = seqNo;
	SendFrame(newtFd, laFrameHead, NULL, 0);
	
	return;
}

int RecvFrame(int newtFd, unsigned char *frame)
{
	char errMesg[] = "Error in reading from Newton device, connection stopped!!";
	int state;
	unsigned char buf;
	unsigned short fcsWord;
	int i;
	
	/* Initialize */
	fcsWord = 0;
	i = 0;
	
	/* Wait for head */
	state = 0;
	while(state < 3) {
		if(read(newtFd, &buf, 1) < 0)
			ErrHandler(errMesg);
		switch(state) {
			case 0:
				if(buf == FrameStart[0])
					state++;
				break;
			case 1:
				if(buf == FrameStart[1])
					state++;
				else
					state = 0;
				break;
			case 2:
				if(buf == FrameStart[2])
					state++;
				else
					state = 0;
				break;
			}
		}
	
	/* Wait for tail */
	state = 0;
	while(state < 2) {
		if(read(newtFd, &buf, 1) < 0)
			ErrHandler(errMesg);
		switch(state) {
			case 0:
				if(buf == '\x10')
					state++;
				else {
					FCSCalc(&fcsWord, buf);
					if(i < MaxHeadLen + MaxInfoLen) {
						frame[i] = buf;
						i++;
						}
					else
						return -1;
					}
				break;
			case 1:
				if(buf == '\x10') {
					FCSCalc(&fcsWord, buf);
					if(i < MaxHeadLen + MaxInfoLen) {
						frame[i] = buf;
						i++;
						}
					else
						return -1;
					state = 0;
					}
				else {
					if(buf == '\x03') {
						FCSCalc(&fcsWord, buf);
						state++;
						}
					else
						return -1;
					}
				break;
			}
		}
		
	/* Check FCS */
	if(read(newtFd, &buf, 1) < 0)
		ErrHandler(errMesg);
	if(fcsWord % 256 != buf)
		return -1;
	if(read(newtFd, &buf, 1) < 0)
		ErrHandler(errMesg);
	if(fcsWord / 256 != buf)
		return -1;

	if(frame[1] == '\x02')
		ErrHandler("Newton device disconnected, connection stopped!!");
	return 0;
}

int WaitLAFrame(int newtFd, uchar seqNo)
{
	uchar frame[MaxHeadLen + MaxInfoLen];

	do {
		while(RecvFrame(newtFd, frame) < 0);
		if(frame[1] == '\x04')
			SendLAFrame(newtFd, frame[2]);
		} while(frame[1] != '\x05');
	if(frame[2] == seqNo)
		return 0;
	else
		return -1;
}

int WaitLDFrame(int newtFd)
{
	char errMesg[] = "Error in reading from Newton device, connection stopped!!";
	int state;
	unsigned char buf;
	unsigned short fcsWord;
	int i;
	
	/* Initialize */
	fcsWord = 0;
	i = 0;
	
	/* Wait for head */
	state = 0;
	while(state < 5) {
		if(read(newtFd, &buf, 1) < 0)
			ErrHandler(errMesg);
		switch(state) {
			case 0:
				if(buf == FrameStart[0])
					state++;
				break;
			case 1:
				if(buf == FrameStart[1])
					state++;
				else
					state = 0;
				break;
			case 2:
				if(buf == FrameStart[2])
					state++;
				else
					state = 0;
				break;
			case 3:
				FCSCalc(&fcsWord, buf);
				state++;
				break;
			case 4:
				if(buf == '\x02') {
					FCSCalc(&fcsWord, buf);
					state++;
					}
				else {
					state = 0;
					fcsWord = 0;
					}
				break;
			}
		}
	
	/* Wait for tail */
	state = 0;
	while(state < 2) {
		if(read(newtFd, &buf, 1) < 0)
			ErrHandler(errMesg);
		switch(state) {
			case 0:
				if(buf == '\x10')
					state++;
				else
					FCSCalc(&fcsWord, buf);
				break;
			case 1:
				if(buf == '\x10') {
					FCSCalc(&fcsWord, buf);
					state = 0;
					}
				else {
					if(buf == '\x03') {
						FCSCalc(&fcsWord, buf);
						state++;
						}
					else
						return -1;
					}
				break;
			}
		}
		
	/* Check FCS */
	if(read(newtFd, &buf, 1) < 0)
		ErrHandler(errMesg);
	if(fcsWord % 256 != buf)
		return -1;
	if(read(newtFd, &buf, 1) < 0)
		ErrHandler(errMesg);
	if(fcsWord / 256 != buf)
		return -1;

	return 0;
}

void ErrHandler(char *errMesg)
{
	fprintf(stderr, "\n");
	fprintf(stderr, errMesg);
	fprintf(stderr, "\n\n");
	exit(0);
}

void SigInt(int sigNo)
{
	if(intNewtFd >= 0) {
		/* Wait for all buffer sent */
		tcdrain(intNewtFd);
		
		SendFrame(intNewtFd, LDFrame, NULL, 0);
		}
	ErrHandler("User interrupted, connection stopped!!");
}
