/* Structure definition */
#define uchar unsigned char
#define ushort unsigned short
typedef struct {
	char devName[256];
	int speed;
	} NewtDevInfo;

/* Constants definition */
#define MaxHeadLen 256
#define MaxInfoLen 256
extern uchar LRFrame[];
extern uchar LDFrame[];

/* Function prototype */
int InitNewtDev(NewtDevInfo *newtDevInfo);
void FCSCalc(ushort *fcsWord, uchar octet);
void SendFrame(int newtFd, uchar *head, uchar *info, int infoLen);
void SendLTFrame(int newtFd, uchar seqNo, uchar *info, int infoLen);
void SendLAFrame(int newtFd, uchar seqNo);
int RecvFrame(int newtFd, uchar *frame);
int WaitLAFrame(int newtFd, uchar seqNo);
int WaitLDFrame(int newtFd);
void ErrHandler(char *errMesg);
void SigInt(int sigNo);
