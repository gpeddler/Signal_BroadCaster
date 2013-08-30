#include <stdio.h>
#include <stdlib.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#define SIGNAL_TERM		4
#define BROADCAST_PORT	5000
#define SIGNAL_PORT		8888
#define SERVER_IP		"172.16.101.83"

using namespace std;

class SignalFlag {
public:
	SignalFlag();
	void signalConnect();
	int getCurrentSignalState ();
	void setSignal();

private:
	int sock;
};

class TrafficAnalyzer {
public:
	enum { TYPE_VIDEO, TYPE_CAM };

	TrafficAnalyzer();
	~TrafficAnalyzer();
	void createConfigure( char** argv, int type ); 
	void run();

private:
	SignalFlag signalFlag;

	/* Image Processing Variable */
	CvHaarClassifierCascade *cascade;
	CvMemStorage            *storage;
	CvCapture *capture;
	IplImage  *frame, *frame_right;
	int input_resize_percent;
	int captureType;
	int raspIdx;

	/* Communication Variable */
	int sock;                         /* Socket */
	int broadcastPermission;          /* Socket opt to set permission to broadcast */
	struct sockaddr_in broadcastAddr; /* Broadcast address */
	char * send_buff;

	void analyze();
	void initUDP();
	void cleanUp();
	void setFrameSize( int per );
	void sendTrafficData( int timestamp, int out_index, int current_signal, int count ); 
	int detectVehicle( IplImage *img ); 
};

TrafficAnalyzer::TrafficAnalyzer() {
	input_resize_percent = 100;
	captureType = -1;
	raspIdx = -1;

	signalFlag.signalConnect();
	initUDP();
}

TrafficAnalyzer::~TrafficAnalyzer() {
	cleanUp();
}

void TrafficAnalyzer::cleanUp() {
	cvDestroyAllWindows();
	cvReleaseImage(&frame);
	cvReleaseImage(&frame_right);
	cvReleaseCapture(&capture);
	cvReleaseHaarClassifierCascade(&cascade);
	cvReleaseMemStorage(&storage);
}

void TrafficAnalyzer::createConfigure( char** argv, int type ) {
	if ( type != TYPE_VIDEO && type != TYPE_CAM ) return;

	raspIdx = atoi( argv[3] );
	cascade = (CvHaarClassifierCascade*) cvLoad(argv[1], 0, 0, 0);
	storage = cvCreateMemStorage(0);
	switch ( type ) {
	case TYPE_VIDEO:
		capture = cvCaptureFromAVI(argv[2]);
		break;
	case TYPE_CAM:
		capture = cvCaptureFromCAM(0);
		break;
	}

	assert(cascade && storage && capture);
}

void TrafficAnalyzer::analyze() {
	
}

void TrafficAnalyzer::run() {
	/* Image processing per frame */
	IplImage* frame1 = cvQueryFrame(capture);
	frame = cvCreateImage(cvSize((int)((frame1->width*input_resize_percent)/100) , (int)((frame1->height*input_resize_percent)/100)), frame1->depth, frame1->nChannels);
	frame_right = cvCreateImage ( cvSize ( frame->width/2, frame->height ), frame->depth, frame->nChannels );

	const int KEY_SPACE  = 32;
	const int KEY_ESC    = 27;

	int key, vehicleCnt, frameCnt, prevState, term;
	key = vehicleCnt = frameCnt = term = 0;
	prevState = signalFlag.getCurrentSignalState();
	do
	{
		/* Get a signal from signal flag as time */
		int currentState = signalFlag.getCurrentSignalState();
		/* When the signal is changed */
		if ( prevState != currentState ) {
			prevState = currentState;
			time_t t;
			time ( &t );	
			if ( currentState != -1 )
			{
				sendTrafficData ( t, raspIdx, currentState, vehicleCnt/frameCnt ); 
			}
			cout << "Changed " << currentState << " & cnt per signal " << vehicleCnt/frameCnt <<  endl;
			vehicleCnt = frameCnt = 0;
		}

		frame1 = cvQueryFrame(capture);

		if(!frame1)
			break;

		cvResize(frame1, frame);

		/* Focus lines on right */
		cvSetImageROI ( frame, cvRect ( frame->width/2, 0, frame->width, frame->height) );
		cvCopy ( frame, frame_right );
		cvResetImageROI( frame );

		/* Detect vechicles and Count */
		vehicleCnt += detectVehicle( frame_right ); 
		frameCnt++;
		term++;

		if ( term == (SIGNAL_TERM*3) && raspIdx == 0 ) {
			term = 0;
			/* Sending signal to receiver */
		}

		key = cvWaitKey(10);
		if(key == KEY_ESC)
			break;

	}while(1);
}

int TrafficAnalyzer::detectVehicle ( IplImage *img )
{
	CvSize img_size = cvGetSize(img);
	CvSeq *object = cvHaarDetectObjects(
		img,
		cascade,
		storage,
		1.1, //1.1,//1.5, //-------------------SCALE FACTOR
		1, //2        //------------------MIN NEIGHBOURS
		0, //CV_HAAR_DO_CANNY_PRUNING
		cvSize(0,0),//cvSize( 30,30), // ------MINSIZE
		img_size //cvSize(70,70)//cvSize(640,480)  //---------MAXSIZE
	);

	std::cout << "Total: " << object->total << " cars" << std::endl;

	return object->total;
/*

  for(int i = 0 ; i < ( object ? object->total : 0 ) ; i++)
  {
    CvRect *r = (CvRect*)cvGetSeqElem(object, i);
    cvRectangle(img,
      cvPoint(r->x, r->y),
      cvPoint(r->x + r->width, r->y + r->height),
      CV_RGB(255, 0, 0), 2, 8, 0);
  }

  cvShowImage(windowName, img);
*/
}

void TrafficAnalyzer::initUDP()
{
	send_buff = (char*)calloc(128, sizeof(char));

	/* Create socket for sending/receiving datagrams */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		assert(1);

	/* Set socket to allow broadcast */
	broadcastPermission = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission, sizeof(broadcastPermission)) < 0 )
		assert(1);

	printf("UDP socket connected\n");

	/* Construct local address structure */
	memset(&broadcastAddr, 0, sizeof(broadcastAddr));			/* Zero out structure */
	broadcastAddr.sin_family		= AF_INET;					/* Internet address family */
	broadcastAddr.sin_addr.s_addr	= INADDR_BROADCAST;			/* Broadcast IP address */
	broadcastAddr.sin_port			= htons( BROADCAST_PORT );	/* Broadcast port */
}

void TrafficAnalyzer::sendTrafficData( int timestamp, int out_index, int current_signal, int count ) 
{
    sprintf ( send_buff, "%d;%d;%d;%d", timestamp, out_index, current_signal, count );
    if (sendto(sock, send_buff, strlen(send_buff), 0, (struct sockaddr *) 
           &broadcastAddr, sizeof(broadcastAddr)) != strlen(send_buff))
        assert(1); /* sendto() sent a different number of bytes than expected */
}

SignalFlag::SignalFlag() {
}

int SignalFlag::getCurrentSignalState()
{
	char message[10] = "get", server_reply[2000];
	if ( sock == -1 )
	{
		 return -1;
	}

	if ( send( sock, message, strlen( message ), 0 ) < 0 )
	{
		return -1;
	}

	if ( recv( sock, server_reply, 2000, 0 ) < 0 )
	{
		return -1;
	}

	return atoi ( server_reply );
}

void SignalFlag::signalConnect()
{
	struct sockaddr_in server;

	/* Create socket */
	sock = socket( AF_INET, SOCK_STREAM, 0 );
	if ( sock == -1 )
	{
		printf("Could not create socket");
	}

	server.sin_addr.s_addr	= inet_addr( SERVER_IP );
	server.sin_family		= AF_INET;
	server.sin_port			= htons( SIGNAL_PORT );

	/* Connect to remote server */
	if ( connect ( sock, (struct sockaddr*)&server, sizeof(server) ) < 0 )
	{
		perror("Connect failed. Error");
		sock = -1;
		return;
	}

	puts("Signal server connected");
}

int main(int argc, char** argv)
{
	TrafficAnalyzer trafficAnalyzer;

	if(argc < 4)
	{
		std::cout << "Usage " << argv[0] << " cascade.xml video.avi raspindex" << std::endl;
		return 0;
	}

	if(argc == 5)
	{
		/* trafficAnalyzer.setFrameSize( atoi( argv[4] ) ); */
	}

	/*	Set the type which will be analyzed. Analyzer supports two types of both video and cam.
		Ready to analyze streaming image by loading haar-like xml. 								*/
	trafficAnalyzer.createConfigure( argv, trafficAnalyzer.TYPE_VIDEO );
	/*	Run */
	trafficAnalyzer.run();

	return 0;
}
