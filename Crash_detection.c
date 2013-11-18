#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <aubio.h>
#include <pthread.h>
#include <signal.h>
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv/cxcore.h"

IplImage* video_frames[30];         //Storage of the video frame

void saveFrame(void* data);         //Video frame is stored in the ring buffer.
void detection_crash(void* data);   //Detection crash sound function
void saveAVI(int sig);              //Stored frames of ring buffer write on avi files

pthread_mutex_t mutx;               //mutex of video_frames_count

int video_frames_count;             //Last count of stored frame
int save_signal_flag;               //1 = called alarm signal
int save_frame_flag;                //1 = called saveAVI function

int main(int argc, char* argv[])
{
    pthread_t p_thread[2];
    int thread_id[2];
    int status;
    int state;
    
    video_frames_count = 0;
    save_frame_flag = 0;
    save_signal_flag = 0;
    
    state = pthread_mutex_init(&mutx,NULL);
    
    if(state){
        printf("mutex init failed\n");
        exit(1);
    }
    
    thread_id[0] = pthread_create(&p_thread[1],NULL,(void *) detection_crash,NULL); //Create thread, for crash detection
    thread_id[1] = pthread_create(&p_thread[0],NULL,(void *) saveFrame,NULL); //Create thread, for save video frame
    
    pthread_join(p_thread[0],(void **)&status);
    pthread_join(p_thread[1],(void **)&status);
    pthread_mutex_destroy(&mutx);
    
    return 0;
}

void saveFrame(void* data)
{
    CvCapture *capture = 0;
    capture = cvCaptureFromCAM(0);   //Usb Webcam
    signal(SIGALRM,saveAVI);         //call saveAVI function after 5second from crash

    if(!capture)
        printf("Could not connect to camera\n");
  
    while(1)
    {
        if(save_frame_flag == 0)
        {
            pthread_mutex_lock(&mutx);
            video_frames[video_frames_count++] = cvCloneImage(cvQueryFrame(capture)); //save frame from usb webcam
            if(video_frames_count == 30)
                video_frames_count = 0;
            pthread_mutex_unlock(&mutx);
        }
    }
    cvReleaseCapture(&capture);
    return;
}

void detection_crash(void* data)
{
    int rc;
	int i = 0, j=0;
	float Frequency = 0;
	int size = 0;
	int dir = 0;
	int detection_count = 0;
	int detection_loop = 0;
    char *buffer;
	sigset_t signal_mask;
	unsigned int val = 44100;
	uint_t win_s = 1024;
	uint_t hop_s = win_s/4;
	uint_t samplerate = 44100;
	uint_t channels = 2;
    
	snd_pcm_uframes_t sound_frames;
    snd_pcm_t *handle;
	snd_pcm_hw_params_t *params;
 	aubio_pitchdetection_mode mode = aubio_pitchm_freq;
	aubio_pitchdetection_type type = aubio_pitch_yinfft;
    
	sigemptyset(&signal_mask);
	sigaddset(&signal_mask,SIGALRM);
	pthread_sigmask(SIG_BLOCK,&signal_mask,NULL);       //SIGALRM BLOCK

	fvec_t * in = new_fvec (hop_s, channels);
	aubio_pitchdetection_t * o = new_aubio_pitchdetection(win_s, hop_s, channels, samplerate, type, mode);

	rc = snd_pcm_open(&handle, "hw:1,0", SND_PCM_STREAM_CAPTURE,SND_PCM_ASYNC);
	if (rc < 0) {
		fprintf(stderr,"unable to open pcm device: %s\n",snd_strerror(rc));
		exit(1);
	}
    
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(handle, params);
	snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_BE);
	snd_pcm_hw_params_set_channels(handle, params, channels);
    
	samplerate = 44100;
	snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
    
	sound_frames = 32;
	rc = snd_pcm_hw_params(handle, params);
    
	if (rc < 0) {
		fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
		exit(1);
	}
    
	snd_pcm_hw_params_get_period_size(params, &sound_frames, &dir);
	size = sound_frames * 4; // 16bit(2byte) /sample, 2 channels
	buffer = (char *) malloc(size);
	snd_pcm_hw_params_get_period_time(params, &samplerate, &dir);
	detection_loop = 500000 / samplerate;
    
	while (1) {
		rc = snd_pcm_readi(handle, buffer, sound_frames);
		if (rc == -EPIPE) {
			fprintf(stderr, "overrun occurred\n");
			snd_pcm_prepare(handle);
		}
		else if (rc < 0) {
			fprintf(stderr,"error from read: %s\n",snd_strerror(rc));
		}
		else if (rc != sound_frames) {
			fprintf(stderr, "short read, read %d sound_frames\n", rc);
		}
		for(i=0 ; i< 256 ; i++)
		{
			in->data[0][i] = (float)((int)buffer[i*4] | ((int)buffer[i*4+1]<<8));
			in->data[1][i] = (float)((int)buffer[i*4+2] |((int)buffer[i*4+3]<<8));
		}
        
		Frequency = aubio_pitchdetection(o,in);     //pitch detection
        
		if(Frequency > 1400 && Frequency <1700)     //crash Frequency
			detection_count++;
        
       		 ++j;
        
		if(j == detection_loop)
		{
			if(detection_count > 10)
            {
				if(save_signal_flag == 0)
				{
					alarm(5);       //call saveAVI function after 5 seconds
					printf("Crash!!\n");
					save_signal_flag = 1;
				}
			}
			detection_count = 0;
            j = 0;
		}
	}
	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	free(buffer);

}



void saveAVI(int sig)
{
	CvVideoWriter *writer;
	int isColor = 1;            //1 is color, 0 is grayscale
	int fps = 3;                //Max fps on raspberry pi
	int i = 0;
    time_t timer;
    struct tm * t;
    char filename[25] = {0,};
    timer = time(NULL);         //get current time
    timer = timer - 10;
    t = localtime(&timer);
    
    sprintf(filename,"%04d%02d%02d_%02d%02d%02d.avi",t->tm_year+1900,t->tm_mon+1,t->tm_mday,t->tm_hour,t->tm_min,t->tm_sec);

	save_frame_flag = 1;
	pthread_mutex_lock(&mutx);
	i = video_frames_count;
	pthread_mutex_unlock(&mutx);

	writer = cvCreateVideoWriter(filename,CV_FOURCC('M','J','P','G'), fps, cvSize(320,240), isColor);
    
    do{
        cvWriteFrame(writer, video_frames[i++]);
        if(i == 30)
            i=0;
    }while(video_frames_count != i);
    
     cvReleaseVideoWriter(&writer);
     save_signal_flag = 0;
     save_frame_flag = 0;
}
