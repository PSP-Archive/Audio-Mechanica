
//                                Audio Mechanica V8d for PSP by Art.

#include <pspsdk.h>
#include <pspuser.h>
#include <pspdisplay.h>
#include <pspkernel.h>
#include <pspmoduleinfo.h>
#include <pspctrl.h>
#include <psputility_usbmodules.h>
#include <pspusb.h>
#include <psputility.h>
#include <psputility_osk.h>
#include <pspusbacc.h>
#include <pspusbcam.h>
#include <pspjpeg.h>
#include <psppower.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <png.h>
#include <pspgu.h>
#include <pspgum.h>
#include <psposk.h>
#include "psp_audio_ext.h"
#include "graphics.h"
#include "main.h"
#include "pspmic.h"
#include "intraFont.h"
#include <psprtc.h>
#include <time.h>
#include "click.h"
#include "ling.h"
#include "headset.h"
#include "gocam.h"
#include "talk.h"
#include "Art.h"
#include "buttons.h"
#include "monit.h"
#include "msrec.h"
#include "back.h"
#include "board.h"
#include "idelay.h"
#include "vact.h"
#include "ram.h"
#include "hpbp.h"
#include "load.h"
#include "message.h"

PSP_MODULE_INFO("Audio_Mechanica", 0, 8, 2);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

// Talkman
#define PSP_USBMIC_PID 0x025b
#define PSP_USBACCBASE_DRIVERNAME "USBAccBaseDriver"
#define PSP_USBMIC_DRIVERNAME "USBMicDriver"
#define INPUT_VOLUME_MAX 0x5fee
#define INPUT_VOLUME INPUT_VOLUME_MAX
// GU
#define BUF_WIDTH (512)
#define SCR_WIDTH (480)
#define SCR_HEIGHT (272)

sample_t* loop_buffer;				//
int pathset;					// path set manualy flag
int samx;					//
int mode;					// recording or monitor mode flag
int wt;						// delay counter
int prn;					// prior record numbers
int increment;					//
int recordnumber;				// file number
int folderexists;				// folder exists flag
int kick = 0;					// kick status outer
int arta = 0x40;				// A
int artr = 0x74;				// r
int artt = 0x74;				// t
int amerror;					// error flag
int sam = 1;					// sample counter
int samlock = 0;				// player stopped
int samsel = 1;					// sample select
int g;						// wave draw variable
int sec;					// seconds variable
int secx;					// seconds compare variable
int secondsx;					//
int seconds;					//
int minutes;					//
int hours;					//
int pyy;					// image coordinates for colour modification
int pxx;					//
char pathx[60];					// pre check path
char fillerx[60];				// character printing array a
char filler[60];				// character printing array b
char fillere[60];				// character printing array c
char recstate[60];				// character printing array d
char fillerm[99];				// character printing array e
char fillerram[60];				// character printing array f
char fillert[20];				// character printing array g
char path[128];					// file path for auto file naming
char spath[128];				// file path without name osk
char extension[5];				// file extension osk
char fullName[256];				// path to html eboot
int del;					// delay counter
int mictype;					// microphone type indicator
unsigned int buf[5];				// free memory display buffers
unsigned int *pbuf = buf;			//
u32 freemem;					// free Memory Stick space Mb buffer
u32 freebytes;					// free memory stick space in bytes
u32 pixel;					//
u32 sample;					//
u32 coolcol;					// constant colour value
int freemb;					//
int fsec;					// file name time variables
int fmn;					//
int fhour;					//
int fyear;					//
int fmonth;					//
int fday;					//
int recording;					// recording flag
int recordindex;				// record index
int echoindex;					// echo index
int delay;					// delay value
int cxbuf;					//
int milisecs;					//
int ecsec;					//
int vox;					// voice acticated mode flag
int sens;					// voice activation sensitivity
int trigger;					// voice activation trigger flag
int menu;					// menu active flag
int reset;					// delete audio thread flag
int titlesize;					// message length
int mchange;					// message change flag
int firsttime;					// first time menu flag
int prxflag;					// flag to load prxes first time only
int mpos;					// scrolling message position
pspTime rtime;					// real time
static u8 work[4096];				// audio buffer
static u8 buffer[32768];			//
unsigned short ubuf[32768];			//
unsigned short dbuf[2048];			//
intraFont* ltn8;				//
unsigned short bigbuffer[7340032];		// 14Mb buffer

void subrectangle(void);
void drawwave(void);
void border(void);
void saveScreen(void);
void errorscreen(void);
void fullscreen(void);
void free_ms_memory(void);
void calcfree(void);
void gettime(void);
void fileexist(void);
u32 ramAvailableLineareMax(void);
void GetRAMFree(void);
void ramscreen(void);
void reset_mode(void);

SceUID amthid;

int exit_callback(int arg1, int arg2, void *common) {
		pspAudioEnd();
		sceKernelExitGame();
		return 0;
}

int CallbackThread(SceSize args, void *argp) {
		int cbid;
		cbid = sceKernelCreateCallback("Exit Callback", (void *) exit_callback, NULL);
		sceKernelRegisterExitCallback(cbid);
		sceKernelSleepThreadCB();
		return 0;
}

int SetupCallbacks(void) {
		int thid = 0;
		thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
		if(thid >= 0) {sceKernelStartThread(thid, 0, 0);}
		return thid;
}

void audioOutputLoopCallback(void* buf, unsigned int length, void *userdata) {
		sample_t* xubuf = (sample_t*) buf;
		int i;
		if (samlock == 1) {
		if (samsel == 1) {
		for (i = 0; i < 1024; i++) {
		xubuf[i].l = click[sam+1] + (click[sam]*256);
		xubuf[i].r = xubuf[i].l;
		sam++;
		sam++;
		if (sam > 2400) {
		xubuf[i].l = 0;
		xubuf[i].r = 0;
		samlock = 0;
		}
		}
		}
		if (samsel == 2) {
		for (i = 0; i < 1024; i++) {
		xubuf[i].l = ling[sam+1] + (ling[sam]*256);
		xubuf[i].r = xubuf[i].l;
		sam++;
		sam++;
		if (sam < 44) {
		xubuf[i].l = 0;
		xubuf[i].r = 0;
		}
		if (sam > 119810) {
		xubuf[i].l = 0;
		xubuf[i].r = 0;
		samlock = 0;
		}
		}
		}
		} // samlock

		if (samlock == 0) {
		int bbb;
		for (bbb = 0; bbb < 1024; bbb++) {
		xubuf[bbb].l = 0xFFFF;
		xubuf[bbb].r = 0xFFFF;
		}
		}

		if (mictype > 3) {
		if (mictype < 7) {
		if (recording == 1) {
		for (i = 0; i < 1024; i++) {
		xubuf[i].l = loop_buffer[i].l;
		xubuf[i].r = loop_buffer[i].r;
		}
		}}}

		if (mictype >8) {
		if (recording == 1) {

		if (recordindex > delay - 1) {
		echoindex = recordindex - delay;
		} else {
		cxbuf = delay - recordindex;
		echoindex = 7331840 - cxbuf;
		}

		for (i = 0; i < 1024; i++) {
		xubuf[i].l = loop_buffer[i].l;
		xubuf[i].r = bigbuffer[echoindex];
		echoindex++;
		}
		}}
}
   
void audioLoopStart() {
		loop_buffer = (sample_t*) malloc(PSP_NUM_AUDIO_SAMPLES * sizeof(sample_t));
		sceKernelDelayThread(200000);	
		pspAudioSetChannelCallback(0, audioOutputLoopCallback, NULL);
}

void audioLoopStop() {
		pspAudioSetChannelCallback(0, NULL, NULL);	
}

int LoadModules() {
		int result = sceUtilityLoadUsbModule(PSP_USB_MODULE_ACC);
		if (result < 0) {
		sprintf(fillere, "Error loading usbacc.prx");
		errorscreen(); amerror = 1;
		return result;
		}
		result = sceUtilityLoadUsbModule(PSP_USB_MODULE_CAM);	
		if (result < 0) {
		sprintf(fillere, "Error loading usbcam.prx");
		errorscreen(); amerror = 1;
		}
		return result;
}

int UnloadModules() {
		int result = sceUtilityUnloadUsbModule(PSP_USB_MODULE_CAM);
		if (result < 0) {
		sprintf(fillere, "Error unloading usbacc.prx");
		errorscreen(); amerror = 1;
		return result;
		}
		result = sceUtilityUnloadUsbModule(PSP_USB_MODULE_ACC);
		if (result < 0) {
		sprintf(fillere, "Error unloading usbacc.prx");
		errorscreen(); amerror = 1;
		}
		return result;
}

int StartUsb() {
		int result = sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
		if (result < 0) {
		sprintf(fillere, "Error starting usbbus driver");
		errorscreen(); amerror = 1;
		return result;
		}
		result = sceUsbStart(PSP_USBACC_DRIVERNAME, 0, 0);
		if (result < 0) {
		sprintf(fillere, "Error starting usbacc driver");
		errorscreen(); amerror = 1;
		return result;
		}
		result = sceUsbStart(PSP_USBCAM_DRIVERNAME, 0, 0);
		if (result < 0) {
		sprintf(fillere, "Error starting usbcam driver");
		errorscreen(); amerror = 1;
		return result;
		}
		result = sceUsbStart(PSP_USBCAMMIC_DRIVERNAME, 0, 0);
		if (result < 0) {
		sprintf(fillere, "Error starting usbcammic driver");
		errorscreen(); amerror = 1;		
		}
		return result;
}

int StopUsb() {
		int result = sceUsbStop(PSP_USBCAMMIC_DRIVERNAME, 0, 0);	
		if (result < 0) {
		sprintf(fillere, "Error stopping usbcammic driver");
		errorscreen(); amerror = 1;
		return result;
		}
		result = sceUsbStop(PSP_USBCAM_DRIVERNAME, 0, 0);
		if (result < 0) {
		sprintf(fillere, "Error stopping usbcam driver");
		errorscreen(); amerror = 1;
		return result;
		}
		result = sceUsbStop(PSP_USBACC_DRIVERNAME, 0, 0);
		if (result < 0) {
		sprintf(fillere, "Error stopping usbacc driver");
		errorscreen(); amerror = 1;
		return result;
		}
		result = sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);
		if (result < 0) {
		sprintf(fillere, "Error stopping usbbus driver");
		errorscreen(); amerror = 1;
		}
		return result;
}

typedef struct {
		u32 ChunkID;
		u32 ChunkSize;
		u32 Format;
		u32 SubChunk1ID;
		u32 SubChunk1Size;
		u16 AudioFormat;
		u16 NumChannels;
		u32 SampleRate;
		u32 ByteRate;
		u16 BlockAlign;
		u16 BitsPerSample;
		u32 SubChunk2ID;
		u32 SubChunk2Size;
} WAVHeader;

		static SceUID startrecord, waitrecord;
		static int stoprecord;

int audio_threade(SceSize args, void *argp) {		// Go!Cam microphone loop experimental thread
		int result;

		PspUsbCamSetupMicParam micparam;
		memset(&micparam, 0, sizeof(micparam));
		micparam.size = sizeof(micparam);
		micparam.gain = 20;
		micparam.frequency = 44100;

		result = sceUsbCamSetupMic(&micparam, work, sizeof(work));
		if (result < 0) {
		sprintf(fillere, "Error in sceUsbCamSetupMic");
		errorscreen(); amerror = 1;
		sceKernelExitDeleteThread(result);
		}

		while (1) {
		sceKernelWaitSema(startrecord, 1, NULL);
		if (reset == 1) {sceKernelExitDeleteThread(0);}
		result = sceUsbCamStartMic();
		if (result < 0) {
		sprintf(fillere, "Error starting microphone");
		errorscreen(); amerror = 1;
		sceKernelExitDeleteThread(result);
		}

		while (!stoprecord) {
		int i;
		samx = 1;
		sceUsbCamReadMicBlocking(buffer, 2048);
		for (i = 0; i < 1024; i++) {
		loop_buffer[i].l = buffer[samx+1] + (buffer[samx]*256);
		loop_buffer[i].r = loop_buffer[i].l;
		samx++; samx++;
		}
		}

		result = sceUsbCamStopMic();
		if (result < 0) {
		sprintf(fillere, "Error stopping microphone");
		errorscreen(); amerror = 1;
		sceKernelExitDeleteThread(result);
		}
		stoprecord = 0;
		sceKernelSignalSema(waitrecord, 1);
		}
}

int audio_threadd(SceSize args, void *argp) {		// Socom microphone loop experimental thread

		while (1) {
		sceKernelWaitSema(startrecord, 1, NULL);
		if (reset == 1) {sceKernelExitDeleteThread(0);}

		while (!stoprecord) {
		sceAudioInputBlocking(1024, 44100, ubuf);
		int i;
		for (i = 0; i < 1024; i++) {
		loop_buffer[i].l = ubuf[i];
		loop_buffer[i].r = ubuf[i];
		}
		}

		stoprecord = 0;
		sceKernelSignalSema(waitrecord, 1);
		}
}

int audio_threadf(SceSize args, void *argp) {		// Talkman microphone loop experimental thread
		sceUsbMicInputInit(0, INPUT_VOLUME, 0);

		while (1) {
		sceKernelWaitSema(startrecord, 1, NULL);
		if (reset == 1) {sceKernelExitDeleteThread(0);}

		while (!stoprecord) {
		sceUsbMicInputBlocking(1024,44100,ubuf);
		int i;
		for (i = 0; i < 1024; i++) {
		loop_buffer[i].l = ubuf[i];
		loop_buffer[i].r = ubuf[i];
		}
		}

		stoprecord = 0;
		sceKernelSignalSema(waitrecord, 1);
		}
}

int audio_thread(SceSize args, void *argp) {		// Socom microphone thread

		while (1) {
		SceUID fd;
		WAVHeader header;
		int datasize;
		sceKernelWaitSema(startrecord, 1, NULL);
		if (reset == 1) {sceKernelExitDeleteThread(0);}
		audioLoopStop();
		if (pathset == 0) {
		sprintf(path,"ms0:/MUSIC/Audio Mechanica/%02d-%02d-%02d_%02d-%02d-%02d.wav",fhour,fmn,fsec,fday,fmonth,fyear);
		} else {
		pathset = 0;
		} // pathset

		fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
		if (fd < 0) {
		sprintf(fillere, "Error opening/creating file");
		errorscreen(); amerror = 1;
		}

		header.ChunkID = 0x46464952; // "RIFF"
		header.ChunkSize = 0; // Not known yet
		header.Format = 0x45564157; // "WAVE"
		header.SubChunk1ID = 0x20746d66; // "fmt "
		header.SubChunk1Size = 16;
		header.AudioFormat = 1; // PCM
		header.NumChannels = 1; // mono
		header.SampleRate = 44100;
		header.ByteRate = header.SampleRate * header.NumChannels * 2;
		header.BlockAlign = header.NumChannels * 2;
		header.BitsPerSample = 16;
		header.SubChunk2ID = 0x61746164; // "data"
		header.SubChunk2Size = 0; // Not known yet

		sceIoWrite(fd, &header, sizeof(header));
		for(wt=0; wt<9; wt++) {sceDisplayWaitVblankStart();} // pause to write header
		datasize = 0;
		while (!stoprecord) {
		sceAudioInputBlocking(2048, 44100, buffer);
		sceIoWrite(fd, buffer, 4096);
		datasize += 4096;
		freebytes = freebytes - 4096;
		}

		header.ChunkSize = datasize + sizeof(WAVHeader) - 8;
		header.SubChunk2Size = datasize;

		sceIoLseek(fd, 0, PSP_SEEK_SET);
		sceIoWrite(fd, &header, sizeof(header));
		sceIoClose(fd);
		audioLoopStart();
		stoprecord = 0;
		sceKernelSignalSema(waitrecord, 1);
		}
}

int audio_threadb(SceSize args, void *argp) {		// Go!Cam microphone thread
		int result;

		PspUsbCamSetupMicParam micparam;
		memset(&micparam, 0, sizeof(micparam));
		micparam.size = sizeof(micparam);
		micparam.gain = 20;
		micparam.frequency = 44100;

		result = sceUsbCamSetupMic(&micparam, work, sizeof(work));
		if (result < 0) {
		sprintf(fillere, "Error in sceUsbCamSetupMic");
		errorscreen(); amerror = 1;
		sceKernelExitDeleteThread(result);
		}

		while (1) {
		SceUID fd;
		WAVHeader header;
		int datasize;
		sceKernelWaitSema(startrecord, 1, NULL);
		if (reset == 1) {sceKernelExitDeleteThread(0);}
		audioLoopStop();
		result = sceUsbCamStartMic();
		if (result < 0)
		{
		sprintf(fillere, "Error starting microphone");
		errorscreen(); amerror = 1;
		sceKernelExitDeleteThread(result);
		}

		if (pathset == 0) {
		sprintf(path,"ms0:/MUSIC/Audio Mechanica/%02d-%02d-%02d_%02d-%02d-%02d.wav",fhour,fmn,fsec,fday,fmonth,fyear);
		} else {
		pathset = 0;
		} // pathset

		fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
		if (fd < 0) {
		sprintf(fillere, "Error opening/creating file");
		errorscreen(); amerror = 1;
		}

		header.ChunkID = 0x46464952; // "RIFF"
		header.ChunkSize = 0; // Not known yet
		header.Format = 0x45564157; // "WAVE"
		header.SubChunk1ID = 0x20746d66; // "fmt "
		header.SubChunk1Size = 16;
		header.AudioFormat = 1; // PCM
		header.NumChannels = 1; // mono
		header.SampleRate = 44100;
		header.ByteRate = header.SampleRate * header.NumChannels * 2;
		header.BlockAlign = header.NumChannels * 2;
		header.BitsPerSample = 16;
		header.SubChunk2ID = 0x61746164; // "data"
		header.SubChunk2Size = 0; // Not known yet

		sceIoWrite(fd, &header, sizeof(header));
		for(wt=0; wt<9; wt++) {sceDisplayWaitVblankStart();} // pause to write header
		datasize = 0;
		while (!stoprecord) {
		sceUsbCamReadMicBlocking(buffer, 4096);
		sceIoWrite(fd, buffer, 4096);
		datasize += 4096;
		freebytes = freebytes - 4096; //
		}

		header.ChunkSize = datasize + sizeof(WAVHeader) - 8;
		header.SubChunk2Size = datasize;

		sceIoLseek(fd, 0, PSP_SEEK_SET);
		sceIoWrite(fd, &header, sizeof(header));
		sceIoClose(fd);

		result = sceUsbCamStopMic();
		if (result < 0) {
		sprintf(fillere, "Error stopping microphone");
		errorscreen(); amerror = 1;
		sceKernelExitDeleteThread(result);
		}

		audioLoopStart();
		stoprecord = 0;
		sceKernelSignalSema(waitrecord, 1);
		}
}

int audio_threadc(SceSize args, void *argp) {		// Talkman buffered microphone thread

		sceUsbMicInputInit(0, INPUT_VOLUME, 0);

		while (1) {
		SceUID fd;
		WAVHeader header;
		int datasize;
		sceKernelWaitSema(startrecord, 1, NULL);
		if (reset == 1) {sceKernelExitDeleteThread(0);}
		audioLoopStop();
		recordindex = 0;
		if (pathset == 0) {
		sprintf(path,"ms0:/MUSIC/Audio Mechanica/%02d-%02d-%02d_%02d-%02d-%02d.wav",fhour,fmn,fsec,fday,fmonth,fyear);
		} else {
		pathset = 0;
		} // pathset

		fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
		if (fd < 0) {
		sprintf(fillere, "Error opening/creating file");
		errorscreen(); amerror = 1;
		}

		header.ChunkID = 0x46464952; // "RIFF"
		header.ChunkSize = 0; // Not known yet
		header.Format = 0x45564157; // "WAVE"
		header.SubChunk1ID = 0x20746d66; // "fmt "
		header.SubChunk1Size = 16;
		header.AudioFormat = 1; // PCM
		header.NumChannels = 1; // mono
		header.SampleRate = 44100;
		header.ByteRate = header.SampleRate * header.NumChannels * 2;
		header.BlockAlign = header.NumChannels * 2;
		header.BitsPerSample = 16;
		header.SubChunk2ID = 0x61746164; // "data"
		header.SubChunk2Size = 0; // Not known yet

		sceIoWrite(fd, &header, sizeof(header));
		for(wt=0; wt<9; wt++) {sceDisplayWaitVblankStart();} // pause to write header
		datasize = 0;

		while (!stoprecord) {
		sceUsbMicInputBlocking(1024,44100,ubuf);
		int i;
		for (i = 0; i < 1024; i++) {
		bigbuffer[recordindex] = ubuf[i];
		recordindex++;
		}

		if (trigger == 0) {
		recordindex = recordindex - 1024;
		} else {
		datasize += 2048;
		freebytes = freebytes - 2048;
		} // trigger

		}
		sceIoWrite(fd, bigbuffer, datasize);

		header.ChunkSize = datasize + sizeof(WAVHeader) - 8;
		header.SubChunk2Size = datasize;

		sceIoLseek(fd, 0, PSP_SEEK_SET);
		sceIoWrite(fd, &header, sizeof(header));
		sceIoClose(fd);
		audioLoopStart();
		stoprecord = 0;
		sceKernelSignalSema(waitrecord, 1);
		}
}

int audio_threadg(SceSize args, void *argp) {		// Socom buffered microphone thread

		while (1) {
		SceUID fd;
		WAVHeader header;
		int datasize;
		sceKernelWaitSema(startrecord, 1, NULL);
		if (reset == 1) {sceKernelExitDeleteThread(0);}
		audioLoopStop();
		recordindex = 0;
		if (pathset == 0) {
		sprintf(path,"ms0:/MUSIC/Audio Mechanica/%02d-%02d-%02d_%02d-%02d-%02d.wav",fhour,fmn,fsec,fday,fmonth,fyear);
		} else {
		pathset = 0;
		} // pathset

		fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
		if (fd < 0) {
		sprintf(fillere, "Error opening/creating file");
		errorscreen(); amerror = 1;
		}

		header.ChunkID = 0x46464952; // "RIFF"
		header.ChunkSize = 0; // Not known yet
		header.Format = 0x45564157; // "WAVE"
		header.SubChunk1ID = 0x20746d66; // "fmt "
		header.SubChunk1Size = 16;
		header.AudioFormat = 1; // PCM
		header.NumChannels = 1; // mono
		header.SampleRate = 44100;
		header.ByteRate = header.SampleRate * header.NumChannels * 2;
		header.BlockAlign = header.NumChannels * 2;
		header.BitsPerSample = 16;
		header.SubChunk2ID = 0x61746164; // "data"
		header.SubChunk2Size = 0; // Not known yet

		sceIoWrite(fd, &header, sizeof(header));
		for(wt=0; wt<9; wt++) {sceDisplayWaitVblankStart();} // pause to write header
		datasize = 0;

		while (!stoprecord) {
		sceAudioInputBlocking(1024, 44100, ubuf);
		int i;
		for (i = 0; i < 1024; i++) {
		bigbuffer[recordindex] = ubuf[i];
		recordindex++;
		}

		if (trigger == 0) {
		recordindex = recordindex - 1024;
		} else {
		datasize += 2048;
		freebytes = freebytes - 2048;
		} // trigger

		}
		sceIoWrite(fd, bigbuffer, datasize);

		header.ChunkSize = datasize + sizeof(WAVHeader) - 8;
		header.SubChunk2Size = datasize;

		sceIoLseek(fd, 0, PSP_SEEK_SET);
		sceIoWrite(fd, &header, sizeof(header));
		sceIoClose(fd);
		audioLoopStart();
		stoprecord = 0;
		sceKernelSignalSema(waitrecord, 1);
		}
}

int audio_threadh(SceSize args, void *argp) {		// Go!Cam buffered microphone thread
		int result;

		PspUsbCamSetupMicParam micparam;
		memset(&micparam, 0, sizeof(micparam));
		micparam.size = sizeof(micparam);
		micparam.gain = 20;
		micparam.frequency = 44100;

		result = sceUsbCamSetupMic(&micparam, work, sizeof(work));
		if (result < 0) {
		sprintf(fillere, "Error in sceUsbCamSetupMic");
		errorscreen(); amerror = 1;
		sceKernelExitDeleteThread(result);
		}

		while (1) {
		SceUID fd;
		WAVHeader header;
		int datasize;
		sceKernelWaitSema(startrecord, 1, NULL);
		if (reset == 1) {sceKernelExitDeleteThread(0);}
		audioLoopStop();
		recordindex = 0;
		result = sceUsbCamStartMic();
		if (result < 0)
		{
		sprintf(fillere, "Error starting microphone");
		errorscreen(); amerror = 1;
		sceKernelExitDeleteThread(result);
		}

		if (pathset == 0) {
		sprintf(path,"ms0:/MUSIC/Audio Mechanica/%02d-%02d-%02d_%02d-%02d-%02d.wav",fhour,fmn,fsec,fday,fmonth,fyear);
		} else {
		pathset = 0;
		} // pathset

		fd = sceIoOpen(path, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
		if (fd < 0) {
		sprintf(fillere, "Error opening/creating file");
		errorscreen(); amerror = 1;
		}

		header.ChunkID = 0x46464952; // "RIFF"
		header.ChunkSize = 0; // Not known yet
		header.Format = 0x45564157; // "WAVE"
		header.SubChunk1ID = 0x20746d66; // "fmt "
		header.SubChunk1Size = 16;
		header.AudioFormat = 1; // PCM
		header.NumChannels = 1; // mono
		header.SampleRate = 44100;
		header.ByteRate = header.SampleRate * header.NumChannels * 2;
		header.BlockAlign = header.NumChannels * 2;
		header.BitsPerSample = 16;
		header.SubChunk2ID = 0x61746164; // "data"
		header.SubChunk2Size = 0; // Not known yet

		sceIoWrite(fd, &header, sizeof(header));
		for(wt=0; wt<9; wt++) {sceDisplayWaitVblankStart();} // pause to write header
		datasize = 0;

		while (!stoprecord) {
		sceUsbCamReadMicBlocking(buffer, 2048);
		int i;
		samx = 1;
		for (i = 0; i < 1024; i++) {
		ubuf[i] = buffer[samx+1] + (buffer[samx]*256);
		bigbuffer[recordindex] = ubuf[i];
		recordindex++;
		samx++; samx++;
		}

		if (trigger == 0) {
		recordindex = recordindex - 1024;
		} else {
		datasize += 2048;
		freebytes = freebytes - 2048;
		} // trigger

		}
		sceIoWrite(fd, bigbuffer, datasize);

		header.ChunkSize = datasize + sizeof(WAVHeader) - 8;
		header.SubChunk2Size = datasize;

		sceIoLseek(fd, 0, PSP_SEEK_SET);
		sceIoWrite(fd, &header, sizeof(header));
		sceIoClose(fd);

		result = sceUsbCamStopMic();
		if (result < 0) {
		sprintf(fillere, "Error stopping microphone");
		errorscreen(); amerror = 1;
		sceKernelExitDeleteThread(result);
		}

		audioLoopStart();
		stoprecord = 0;
		sceKernelSignalSema(waitrecord, 1);
		}
}

int audio_threadk(SceSize args, void *argp) {		// Talkman microphone echo thread

		sceUsbMicInputInit(0, INPUT_VOLUME, 0);

		while (1) {
		sceKernelWaitSema(startrecord, 1, NULL);
		if (reset == 1) {sceKernelExitDeleteThread(0);}
		recordindex = 0;


		while (!stoprecord) {
		sceUsbMicInputBlocking(1024,44100,ubuf);
		int i;
		for (i = 0; i < 1024; i++) {
		loop_buffer[i].l = ubuf[i];
		bigbuffer[recordindex] = ubuf[i];
		dbuf[i] = bigbuffer[echoindex+i];
		recordindex++;
		}
		if (recordindex > 7331840) {recordindex = 0;}	// loop reel
		}

		stoprecord = 0;
		sceKernelSignalSema(waitrecord, 1);
		}
}

int audio_threadi(SceSize args, void *argp) {		// Socom microphone echo thread

		while (1) {
		sceKernelWaitSema(startrecord, 1, NULL);
		if (reset == 1) {sceKernelExitDeleteThread(0);}
		recordindex = 0;

		while (!stoprecord) {
		sceAudioInputBlocking(1024, 44100, ubuf);
		int i;
		for (i = 0; i < 1024; i++) {
		loop_buffer[i].l = ubuf[i];
		bigbuffer[recordindex] = ubuf[i];
		dbuf[i] = bigbuffer[echoindex+i];
		recordindex++;
		}
		if (recordindex > 7331840) {recordindex = 0;}	// loop reel
		}

		stoprecord = 0;
		sceKernelSignalSema(waitrecord, 1);
		}
}

int audio_threadj(SceSize args, void *argp) {		// Go!Cam microphone echo thread
		int result;

		PspUsbCamSetupMicParam micparam;
		memset(&micparam, 0, sizeof(micparam));
		micparam.size = sizeof(micparam);
		micparam.gain = 20;
		micparam.frequency = 44100;

		result = sceUsbCamSetupMic(&micparam, work, sizeof(work));
		if (result < 0) {
		sprintf(fillere, "Error in sceUsbCamSetupMic");
		errorscreen(); amerror = 1;
		sceKernelExitDeleteThread(result);
		}

		while (1) {
		sceKernelWaitSema(startrecord, 1, NULL);
		if (reset == 1) {sceKernelExitDeleteThread(0);}
		recordindex = 0;
		result = sceUsbCamStartMic();
		if (result < 0)
		{
		sprintf(fillere, "Error starting microphone");
		errorscreen(); amerror = 1;
		sceKernelExitDeleteThread(result);
		}


		while (!stoprecord) {
		sceUsbCamReadMicBlocking(buffer, 2048);
		int i;
		samx = 1;
		for (i = 0; i < 1024; i++) {
		loop_buffer[i].l = buffer[samx+1] + (buffer[samx]*256);
		bigbuffer[recordindex] = loop_buffer[i].l;
		dbuf[i] = bigbuffer[echoindex+i];
		recordindex++;
		samx++; samx++;
		}
		if (recordindex > 7331840) {recordindex = 0;}	// loop reel
		}

		result = sceUsbCamStopMic();
		if (result < 0) {
		sprintf(fillere, "Error stopping microphone");
		errorscreen(); amerror = 1;
		sceKernelExitDeleteThread(result);
		}

		stoprecord = 0;
		sceKernelSignalSema(waitrecord, 1);
		}
}

int eblStartFromMs(char * filename);

		Image * Art;
		Image * iheadset;
		Image * igocam;
		Image * italkman;
		Image * ibuttons;
		Image * imonitor;
		Image * imemory;
		Image * background;
		Image * iram;
		Image * iecho;
		Image * ivoice;

int main() {

		kick = artr + artt + arta;
		if (kick == 0x128) {
		artr--;
		Artpic[5] = 0x64;
		Artpic[25] = 0x6D;
		Artpic[37] = 0x1D;
		Artpic[12] = 0x27;
		Artpic[7] = 0x64;
		Artpic[26] = 0x6E;
		arta++;
		Artpic[0] = 0xE7;
		Artpic[6] = 0x74;
		artr--;
		Artpic[1] = 0x3E;
		int xor;
		for(xor=0; xor<316; xor++) {
		Artpic[xor] = Artpic[xor] ^ 0x6E;	// simple xor decrypt Art logo graphic
		}
		} // kick

		Art = loadImageMemory(Artpic, 316);
		iheadset = loadImageMemory(headset, 28799);
		igocam = loadImageMemory(gocam, 55281);
		italkman = loadImageMemory(talk, 17641);
		ibuttons = loadImageMemory(buttons, 6554);
		imonitor = loadImageMemory(monit, 8917);
		imemory = loadImageMemory(msrec, 23384);
		background = loadImageMemory(back, 4582);
		iram = loadImageMemory(board, 41713);
		iecho = loadImageMemory(idelay, 12459);
		ivoice = loadImageMemory(vact, 41819);

		intraFontInit();
		ltn8 = intraFontLoad("flash0:/font/ltn8.pgf",INTRAFONT_CACHE_ASCII); 
		if(!ltn8) {sceKernelExitGame();}
		initGraphics();
		sceDisplayWaitVblankStart();
		fileexist();

	//	scePowerSetClockFrequency(333, 333, 166);	// set clock frequencies

		pyy = 1;
		pxx = 1;
		pixel = getPixelImage(pxx, pyy, Art);
		sample = pixel;
		for (pyy=0; pyy<57; pyy++) { // image height
		for (pxx=0; pxx<100; pxx++) { // image width
		pixel = getPixelImage(pxx, pyy, Art);
		if (pixel == sample) {
		putPixelImage(0x00000000, pxx, pyy, Art);
		} else {
		putPixelImage(0xFFEFDDCA, pxx, pyy, Art);
		}
		} // width
		} // height

		SetupCallbacks();
		loop_buffer = NULL;
		pspAudioInit();
		sceDisplayWaitVblankStart();
		audioLoopStart();
		sceDisplayWaitVblankStart();
		free_ms_memory();	
		sceDisplayWaitVblankStart();

		sprintf(spath, "ms0:/MUSIC/Audio Mechanica/");
		sprintf(extension, ".wav");

		titlesize = strlen(message);

		message[titlesize-1] = 0x20;
		message[titlesize-2] = 0x20;
		message[titlesize-3] = 0x20;
		message[titlesize-4] = 0x20;
		message[titlesize-5] = 0x20;
		message[titlesize-6] = 0x20;
		message[titlesize-7] = 0x20;
		message[titlesize-8] = 0x20;

		delay = 10240;	// set default delay
		sens = 100;	// set default sensitivity
		menu = 1;	// set menu active flag
		mpos = 480;	// set initial scroll position

		sam = 1; // set sample address counter
		samlock = 1; // re-enable player routine
		samsel = 2; // select ling ling sample

		while (1) {
		clearScreen(0);			// display intro screen
		sprintf(fillerx, "Audio Mechanica V8d");
		blitImageToScreen(0, 7, 100, 50, Art, 183,65);
		guStart();
		intraFontSetStyle(ltn8, 2.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 35, fillerx);
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 55, "Audio Recorder for Microphone Headset, Talkman and Go!Cam");
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 120, "01/12/08");
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 145, "Intrafont library by BenHur");
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 160, "Onscreen Keyboard library by Peb");

		if (mchange == 0) {
		intraFontSetStyle(ltn8, 1.2f,0xFFEFDDCA,0,INTRAFONT_ALIGN_LEFT);
		intraFontPrint(ltn8, mpos, 195, message);
		mpos--;
		if (mpos < -2482) {mpos = 480; mchange = 1;}
		} else {
		intraFontSetStyle(ltn8, 1.1f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 195, "Press CROSS to Continue, or RIGHT to check update page");
		} // mchange

		sceGuFinish();
		sceGuSync(0,0);

		drawLineScreen(5, 5, 475, 5, 0xFF555555);
		blitImageToScreen(76, 0, 174, 44, ibuttons, 192,215);

		fillScreenRect(0xFF000000, 0, 167, 5, 100);	// mask sides of scroll text
		fillScreenRect(0xFF000000, 476, 167, 4, 100);

		border();
		sceDisplayWaitVblankStart();
		flipScreen();

		SceCtrlData xpad;
		sceCtrlReadBufferPositive(&xpad, 1);

		if (xpad.Buttons & PSP_CTRL_CROSS) {
		sam = 1; // set sample address counter
		samlock = 1; // re-enable player routine
		samsel = 1; // select click sample
		break;
		}
		if (xpad.Buttons & PSP_CTRL_TRIANGLE) {
		sam = 1; // set sample address counter
		samlock = 1; // re-enable player routine
		samsel = 1; // select click sample
		ramscreen();
		}
		if (xpad.Buttons & PSP_CTRL_RIGHT) {
		sam = 1; // set sample address counter
		samlock = 1; // re-enable player routine
		samsel = 1; // select click sample
		getcwd(fullName,200); 
		strcat(fullName,"/HTML.PBP");
		pspSdkLoadStartModule("./ebootloader.prx", PSP_MEMORY_PARTITION_KERNEL);
		eblStartFromMs(fullName);
		} // right
		}
		for(del=0; del<30; del++) {sceDisplayWaitVblankStart();} // pause for button debounce

		SceCtrlData pad, oldpad;
		recording = 0;
		mictype = 1;			// set initial microphone hardware type

		while (1) {
		clearScreen(0);			// display option screen
		guStart();
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 10, fillerx);

		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		if (mictype == 1) {intraFontPrint(ltn8, 240, 50, "PSP-270x Headset");}
		if (mictype == 2) {intraFontPrint(ltn8, 240, 50, "PSP-300 Go!Cam");}
		if (mictype == 3) {intraFontPrint(ltn8, 240, 50, "PSP-240 Talkman");}
		sceGuFinish();
		sceGuSync(0,0);

		blitImageToScreen(0, 0, 250, 44, ibuttons, 116,215);

		if (mictype == 1) {
		blitImageToScreen(0, 0, 117, 147, igocam, 357,60);
		blitImageToScreen(0, 0, 214, 147, iheadset, 133,60);
		blitImageToScreen(98, 0, 117, 147, italkman, 7,60);
		}
		if (mictype == 2) {
		blitImageToScreen(0, 0, 117, 147, italkman, 357,60);
		blitImageToScreen(0, 0, 214, 147, igocam, 133,60);
		blitImageToScreen(98, 0, 117, 147, iheadset, 7,60);
		}
		if (mictype == 3) {
		blitImageToScreen(0, 0, 117, 147, iheadset, 357,60);
		blitImageToScreen(0, 0, 214, 147, italkman, 133,60);
		blitImageToScreen(98, 0, 117, 147, igocam, 7,60);
		}
		border();
		sceDisplayWaitVblankStart();
		flipScreen();

		sceRtcGetCurrentClockLocalTime(&rtime);	// get the time from PSP real time clock
		secx = sec;
		sec = rtime.seconds;
		calcfree();

		sceCtrlReadBufferPositive(&pad, 1);

		if (pad.Buttons & PSP_CTRL_RIGHT) {
		sam = 1; // set sample address counter
		samlock = 1; // re-enable player routine
		samsel = 1; // select click sample
		mictype++;
		if (mictype > 3) {mictype = 1;}
		for(del=0; del<3; del++) {sceDisplayWaitVblankStart();}
		}

		if (pad.Buttons & PSP_CTRL_LEFT) {
		sam = 1; // set sample address counter
		samlock = 1; // re-enable player routine
		samsel = 1; // select click sample
		mictype--;
		if (mictype < 1) {mictype = 3;}
		for(del=0; del<3; del++) {sceDisplayWaitVblankStart();}
		}

		if (pad.Buttons & PSP_CTRL_CROSS) {
		sam = 1; // set sample address counter
		samlock = 1; // re-enable player routine
		samsel = 1; // select click sample
		clearScreen(0);
		guStart();
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 10, fillerx);
		sceGuFinish();
		sceGuSync(0,0);
		border();
		sceDisplayWaitVblankStart();
		flipScreen();
		if (mictype == 1) {sceAudioInputInit(0, 4096, 0);}
		break;
		} // cross

		sceKernelDelayThread(50000);
		}

		for(del=0; del<30; del++) {sceDisplayWaitVblankStart();} // pause for button debounce

		while (1) {

		if (menu == 1) {
		while (1) {

		clearScreen(0);			// display option screen
		guStart();
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 10, fillerx);

		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		if (mictype == 1) {intraFontPrint(ltn8, 240, 35, "PSP-270x Headset");}
		if (mictype == 2) {intraFontPrint(ltn8, 240, 35, "PSP-300 Go!Cam");}
		if (mictype == 3) {intraFontPrint(ltn8, 240, 35, "PSP-240 Talkman");}

		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		if (mode == 0) {
		if (mictype == 3) {
		intraFontPrint(ltn8, 240, 50, "Buffered Recording Mode");
		} else {
		intraFontPrint(ltn8, 240, 50, "Direct Recording Mode");
		} // mictype
		} // mode
		if (mode == 1) {intraFontPrint(ltn8, 240, 50, "Monitor Loop Mode");}
		if (mode == 2) {
		if (mictype == 3) {
		intraFontPrint(ltn8, 240, 50, "Echo Loop Mode");
		} else {
		intraFontPrint(ltn8, 240, 50, "Buffered Recording Mode");
		} // mictype
		} // mode

		if (mode == 3) {
		if (mictype == 3) {
		intraFontPrint(ltn8, 240, 50, "Voice Activated Buffered Recording Mode");
		} else {
		intraFontPrint(ltn8, 240, 50, "Echo Loop Mode");
		} // mictype
		} // mode

		if (mode == 4) {
		intraFontPrint(ltn8, 240, 50, "Voice Activated Buffered Recording Mode");
		} // mode

		sceGuFinish();
		sceGuSync(0,0);
		blitImageToScreen(0, 0, 250, 44, ibuttons, 116,215);

		if (mictype < 3) {
		if (mode == 0) {
		blitImageToScreen(0, 0, 117, 147, imonitor, 357,60);
		blitImageToScreen(0, 0, 214, 147, imemory, 133,60);
		blitImageToScreen(98, 0, 117, 147, ivoice, 7,60);
		}
		if (mode == 1) {
		blitImageToScreen(0, 0, 117, 147, iram, 357,60);
		blitImageToScreen(0, 0, 214, 147, imonitor, 133,60);
		blitImageToScreen(98, 0, 117, 147, imemory, 7,60);
		}
		if (mode == 2) {
		blitImageToScreen(0, 0, 117, 147, iecho, 357,60);
		blitImageToScreen(0, 0, 214, 147, iram, 133,60);
		blitImageToScreen(98, 0, 117, 147, imonitor, 7,60);
		}
		if (mode == 3) {
		blitImageToScreen(0, 0, 117, 147, ivoice, 357,60);
		blitImageToScreen(0, 0, 214, 147, iecho, 133,60);
		blitImageToScreen(98, 0, 117, 147, iram, 7,60);
		}
		if (mode == 4) {
		blitImageToScreen(0, 0, 117, 147, imemory, 357,60);
		blitImageToScreen(0, 0, 214, 147, ivoice, 133,60);
		blitImageToScreen(98, 0, 117, 147, iecho, 7,60);
		}
		} else {
		if (mode == 0) {
		blitImageToScreen(0, 0, 117, 147, imonitor, 357,60);
		blitImageToScreen(0, 0, 214, 147, iram, 133,60);
		blitImageToScreen(98, 0, 117, 147, ivoice, 7,60);
		}
		if (mode == 1) {
		blitImageToScreen(0, 0, 117, 147, iecho, 357,60);
		blitImageToScreen(0, 0, 214, 147, imonitor, 133,60);
		blitImageToScreen(98, 0, 117, 147, iram, 7,60);
		}
		if (mode == 2) {
		blitImageToScreen(0, 0, 117, 147, ivoice, 357,60);
		blitImageToScreen(0, 0, 214, 147, iecho, 133,60);
		blitImageToScreen(98, 0, 117, 147, imonitor, 7,60);
		}
		if (mode == 3) {
		blitImageToScreen(0, 0, 117, 147, iram, 357,60);
		blitImageToScreen(0, 0, 214, 147, ivoice, 133,60);
		blitImageToScreen(98, 0, 117, 147, iecho, 7,60);
		}
		}

		border();
		sceDisplayWaitVblankStart();
		flipScreen();

		sceRtcGetCurrentClockLocalTime(&rtime);	// get the time from PSP real time clock
		secx = sec;
		sec = rtime.seconds;
		calcfree();

		sceCtrlReadBufferPositive(&pad, 1);

		if (pad.Buttons & PSP_CTRL_RIGHT) {
		if (mictype < 3) {
		sam = 1; // set sample address counter
		samlock = 1; // re-enable player routine
		samsel = 1; // select click sample
		mode++;
		if (mode > 4) {mode = 0;}
		} else {
		sam = 1; // set sample address counter
		samlock = 1; // re-enable player routine
		samsel = 1; // select click sample
		mode++;
		if (mode > 3) {mode = 0;}
		} // mictype
		for(del=0; del<3; del++) {sceDisplayWaitVblankStart();}
		}

		if (pad.Buttons & PSP_CTRL_LEFT) {
		if (mictype < 3) {
		sam = 1; // set sample address counter
		samlock = 1; // re-enable player routine
		samsel = 1; // select click sample
		mode--;
		if (mode < 0) {mode = 4;}
		} else {
		sam = 1; // set sample address counter
		samlock = 1; // re-enable player routine
		samsel = 1; // select click sample
		mode--;
		if (mode < 0) {mode = 3;}
		} // mictype
		for(del=0; del<3; del++) {sceDisplayWaitVblankStart();}
		}

		if (pad.Buttons & PSP_CTRL_CROSS) {
		sam = 1; // set sample address counter
		samlock = 1; // re-enable player routine
		samsel = 1; // select click sample
		clearScreen(0);
		guStart();
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 10, fillerx);
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 135, filler);
		sceGuFinish();
		sceGuSync(0,0);
		border();
		sceDisplayWaitVblankStart();
		flipScreen();
		sprintf(filler, "     ");	// clear buffer
		if (mode == 0) {
		break;
		} // mode
		if (mode == 1) {
		if (mictype == 1) {mictype = 4;break;}
		if (mictype == 2) {mictype = 5;break;}
		if (mictype == 3) {mictype = 6;break;}
		} // mode
		if (mode == 2) {
		if (mictype == 1) {mictype = 7;break;}
		if (mictype == 2) {mictype = 8;break;}
		if (mictype == 3) {mictype =11;break;}
		} // mode
		if (mode == 3) {
		if (mictype == 1) {mictype = 9;break;}
		if (mictype == 2) {mictype =10;break;}
		if (mictype == 3) {vox = 1; mode = 0; break;}
		} // mode
		if (mode == 4) {
		if (mictype == 1) {mictype = 7; mode = 2; vox = 1; break;}
		if (mictype == 2) {mictype = 8; mode = 2; vox = 1; break;}
		} // mode
		} // cross
		sceKernelDelayThread(50000);
		}

		sceRtcGetCurrentClockLocalTime(&rtime);	// get the time from PSP real time clock
		secx = sec;
		sec = rtime.seconds;
		calcfree();

		for(del=0; del<30; del++) {sceDisplayWaitVblankStart();} // pause screen

		if (prxflag == 0) {		// load USB prxes only the first menu session
		if (mictype == 3 || mictype == 6 || mictype == 11) {
		sceUtilityLoadUsbModule(PSP_USB_MODULE_ACC);
		sceUtilityLoadUsbModule(PSP_USB_MODULE_MIC);
		sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
		sceUsbStart(PSP_USBACCBASE_DRIVERNAME, 0, 0);
		sceUsbStart(PSP_USBMIC_DRIVERNAME, 0, 0);

		if (sceUsbActivate(PSP_USBMIC_PID) < 0) {
		sprintf(fillere, "Error activating the Microphone");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}

		while (1) {
		if ((sceUsbGetState() & 0xF) == PSP_USB_CONNECTION_ESTABLISHED) {
		break;
		} else {
		clearScreen(0);			// display waiting screen
		guStart();
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 10, fillerx);
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 125, "Waiting for USB hardware...");
		sceGuFinish();
		sceGuSync(0,0);
		border();
		sceDisplayWaitVblankStart();
		flipScreen();
		}
		sceKernelDelayThread(50000);
		}
		} // mictype

		if (mictype == 2 || mictype == 5 || mictype == 8 || mictype == 10) {
		if (LoadModules() < 0)
		sceKernelSleepThread();
		if (StartUsb() < 0)
		sceKernelSleepThread();
		if (sceUsbActivate(PSP_USBCAM_PID) < 0) {
		sprintf(fillere, "Error activating the camera");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}
		while (1) {
		if ((sceUsbGetState() & 0xF) == PSP_USB_CONNECTION_ESTABLISHED) {
		break;
		} else {
		clearScreen(0);			// display waiting screen
		guStart();
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 10, fillerx);
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 125, "Waiting for USB hardware...");
		sceGuFinish();
		sceGuSync(0,0);
		border();
		sceDisplayWaitVblankStart();
		flipScreen();
		}
		sceKernelDelayThread(50000);
		}
		} // mictype
		} // prxflag

		if (firsttime == 0) {
		startrecord = sceKernelCreateSema("StartRecordSema", 0, 0, 1, NULL);
		if (startrecord < 0) {
		sprintf(fillere, "Cannot create semaphore");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}

		waitrecord = sceKernelCreateSema("WaitRecordSema", 0, 0, 1, NULL);
		if (waitrecord < 0) {
		sprintf(fillere, "Cannot create semaphore");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}
		firsttime = 1;
		} // firsttime
		if (mictype == 1) {
		amthid = sceKernelCreateThread("audio_thread", audio_thread, 16, 256*1024, 0, NULL);
		if (amthid < 0) {
		sprintf(fillere, "Cannot create audio thread");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}

		if (sceKernelStartThread(amthid, 0, NULL) < 0) {
		sprintf(fillere, "Cannot start audio thread");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}
		} // mictype

		if (mictype == 2) {
		amthid = sceKernelCreateThread("audio_threadb", audio_threadb, 16, 256*1024, 0, NULL);
		if (amthid < 0) {
		sprintf(fillere, "Cannot create audio thread");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}

		if (sceKernelStartThread(amthid, 0, NULL) < 0) {
		sprintf(fillere, "Cannot start audio thread");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}
		} // mictype

		if (mictype == 7) {
		amthid = sceKernelCreateThread("audio_threadg", audio_threadg, 16, 256*1024, 0, NULL);
		if (amthid < 0) {
		sprintf(fillere, "Cannot create audio thread");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}

		if (sceKernelStartThread(amthid, 0, NULL) < 0) {
		sprintf(fillere, "Cannot start audio thread");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}
		} // mictype

		if (mictype == 8) {
		amthid = sceKernelCreateThread("audio_threadh", audio_threadh, 16, 256*1024, 0, NULL);
		if (amthid < 0) {
		sprintf(fillere, "Cannot create audio thread");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}

		if (sceKernelStartThread(amthid, 0, NULL) < 0) {
		sprintf(fillere, "Cannot start audio thread");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}
		} // mictype

		if (mictype == 3) {
		amthid = sceKernelCreateThread("audio_threadc", audio_threadc, 16, 256*1024, 0, NULL);
		if (amthid < 0) {
		sprintf(fillere, "Cannot create audio thread");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}

		if (sceKernelStartThread(amthid, 0, NULL) < 0) {
		sprintf(fillere, "Cannot start audio thread");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}
		} // mictype

		if (mictype == 6) {
		amthid = sceKernelCreateThread("audio_threadf", audio_threadf, 16, 256*1024, 0, NULL);
		if (amthid < 0) {
		sprintf(fillere, "Cannot create audio thread");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}

		if (sceKernelStartThread(amthid, 0, NULL) < 0) {
		sprintf(fillere, "Cannot start audio thread");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}
		} // mictype

		if (mictype == 4) {
		amthid = sceKernelCreateThread("audio_threadd", audio_threadd, 16, 256*1024, 0, NULL);
		if (amthid < 0) {
		sprintf(fillere, "Cannot create audio thread");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}

		if (sceKernelStartThread(amthid, 0, NULL) < 0) {
		sprintf(fillere, "Cannot start audio thread");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}
		} // mictype

		if (mictype == 5) {
		amthid = sceKernelCreateThread("audio_threade", audio_threade, 16, 256*1024, 0, NULL);
		if (amthid < 0) {
		sprintf(fillere, "Cannot create audio thread");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}

		if (sceKernelStartThread(amthid, 0, NULL) < 0) {
		sprintf(fillere, "Cannot start audio thread");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}
		} // mictype

		if (mictype == 9) {
		amthid = sceKernelCreateThread("audio_threadi", audio_threadi, 16, 256*1024, 0, NULL);
		if (amthid < 0) {
		sprintf(fillere, "Cannot create audio thread");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}

		if (sceKernelStartThread(amthid, 0, NULL) < 0) {
		sprintf(fillere, "Cannot start audio thread");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}
		} // mictype

		if (mictype == 10) {
		amthid = sceKernelCreateThread("audio_threadj", audio_threadj, 16, 256*1024, 0, NULL);
		if (amthid < 0) {
		sprintf(fillere, "Cannot create audio thread");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}

		if (sceKernelStartThread(amthid, 0, NULL) < 0) {
		sprintf(fillere, "Cannot start audio thread");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}
		} // mictype

		if (mictype == 11) {
		amthid = sceKernelCreateThread("audio_threadk", audio_threadk, 16, 256*1024, 0, NULL);
		if (amthid < 0) {
		sprintf(fillere, "Cannot create audio thread");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}

		if (sceKernelStartThread(amthid, 0, NULL) < 0) {
		sprintf(fillere, "Cannot start audio thread");
		errorscreen(); amerror = 1;
		sceKernelSleepThread();
		}
		mode = 3;
		} // mictype

		clearScreen(0);
		guStart();
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 10, fillerx);
		sceGuFinish();
		sceGuSync(0,0);
		border();
		sceDisplayWaitVblankStart();
		flipScreen();
		for(del=0; del<8; del++) {sceDisplayWaitVblankStart();} // pause screen
		sprintf(recstate, "STOPPED");
		menu = 0;		// deactivate menu
		} // menu

		oldpad.Buttons = 0xFFFFFFFF;

		clearScreen(0);
		guStart();
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 10, fillerx);
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 140, recstate);

		if (mode != 1) {
		if (mictype < 9) {
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);		
		intraFontPrint(ltn8, 58, 243, "Free Space:");
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 58, 258, fillerm);
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);		
		intraFontPrint(ltn8, 424, 243, "Time:");
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 424, 258, fillert);
		} else {
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);		
		intraFontPrint(ltn8, 58, 243, "Echo Delay:");
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 58, 258, fillerm);
		} // mictype
		} // mode

		sceGuFinish();
		sceGuSync(0,0);
		border();

		if (recording == 0) {
		if (mode == 0 || mode == 2) {
		blitImageToScreen(76, 0, 174, 44, ibuttons, 192,215);	// cross and right for file naming
		} else {
		blitImageToScreen(75, 0, 84, 44, ibuttons, 191,215);
		} // mode
		} else {

		if (mode == 3 || vox == 1) {
		blitImageToScreen(0, 0, 250, 44, ibuttons, 116,215);	// left and right for delay or vox
		} else {
		blitImageToScreen(75, 0, 84, 44, ibuttons, 191,215);
		} // mode
		} // recording

		drawwave();
		sceDisplayWaitVblankStart();
		flipScreen();

		if (recording == 1) {
		calcfree();
		if (mictype < 9) {
		if (recordindex > 7331840) {	// 14 Mb buffer
		sam = 1; // set sample address counter
		samlock = 1; // re-enable player routine
		samsel = 1; // select click sample
		stoprecord = 1;
		sceKernelWaitSema(waitrecord, 1, NULL);
		recording = 0;
		sprintf(recstate, "STOPPED");
		for(del=0; del<4096; del++) {buffer[del] = 0;} // clear audio buffer
		sprintf(fillere, "RAM buffer is full!");
		fullscreen();
		for(del=0; del<1024; del++) {ubuf[del] = 0;}
		} // recordindex
		} // mictype

		if (mode == 2 || mictype == 3) {
		if (freemb < 2) {
		sam = 1; // set sample address counter
		samlock = 1; // re-enable player routine
		samsel = 1; // select click sample
		stoprecord = 1;
		sceKernelWaitSema(waitrecord, 1, NULL);
		recording = 0;
		sprintf(recstate, "STOPPED");
		for(del=0; del<4096; del++) {buffer[del] = 0;} // clear audio buffer
		for(del=0; del<4096; del++) {ubuf[del] = 0;} 
		sprintf(fillere, "Low Memory Stick space!");
		errorscreen();
		for(del=0; del<1024; del++) {ubuf[del] = 0;}
		} // freemb
		} // mode-mictype

		if (freemb < 1) {
		sam = 1; // set sample address counter
		samlock = 1; // re-enable player routine
		samsel = 1; // select click sample
		stoprecord = 1;
		sceKernelWaitSema(waitrecord, 1, NULL);
		recording = 0;
		sprintf(recstate, "STOPPED");
		for(del=0; del<4096; del++) {buffer[del] = 0;} // clear audio buffer
		for(del=0; del<4096; del++) {ubuf[del] = 0;} 
		sprintf(fillere, "Low Memory Stick space!");
		errorscreen();
		} // freemb
		} // recording

		sceCtrlPeekBufferPositive(&pad, 1);
		if (pad.Buttons != oldpad.Buttons) {

		if (pad.Buttons & PSP_CTRL_HOME) {
		if (recording == 1) {
		sam = 1; // set sample address counter
		samlock = 1; // re-enable player routine
		samsel = 1; // select click sample
		stoprecord = 1;
		sceKernelWaitSema(waitrecord, 1, NULL);
		recording = 0;
		sprintf(recstate, "STOPPED");
		for(del=0; del<4096; del++) {buffer[del] = 0;} // clear audio buffer
		for(del=0; del<4096; del++) {ubuf[del] = 0;} 
		for(del=0; del<1024; del++) 
		loop_buffer[del].l = 0;
		loop_buffer[del].r = 0;
		}

		} // recording
		}

		if (recording == 0) {
		if (mode == 0 || mode == 2) {
		if (pad.Buttons & PSP_CTRL_RIGHT) {
		sam = 1; // set sample address counter
		samlock = 1; // re-enable player routine
		samsel = 1; // select click sample
		for(wt=0; wt<3; wt++) {sceDisplayWaitVblankStart();} // pause
		char intext[128] = "Name";
		char desc[128] = "Enter File Name";
		char input[128]="";

		SonyOskPng(input,"oskbg",intext,desc,AUTO_LANGUAGE_SELECT);	// Peb's new OSK library command

		pathset = 1;
		if (strlen(input) == 0) {pathset = 0;}	// disreguard if osk was exited
		sprintf (path, "%s" ,spath);
		strcat (path, input);			// add name to path string
		strcat (path, extension);		// add extension to path string
		initGraphics();
		} // right
		} // mode
		} else {
		if (vox == 1) {
		if (pad.Buttons & PSP_CTRL_RIGHT) {
		sens++; sens ++;
		if (sens > 259) {sens = 260;}
		for(del=0; del<3; del++) {sceDisplayWaitVblankStart();}
		} // right
		if (pad.Buttons & PSP_CTRL_LEFT) {
		sens--; sens--;
		if (sens < 11) {sens = 10;}
		for(del=0; del<3; del++) {sceDisplayWaitVblankStart();}
		} // left
		} // vox
		} // recording

		if (mictype > 8) {
		if (pad.Buttons & PSP_CTRL_RIGHT) {
		delay = delay + 1024;
		if (delay > 1843200) {delay = 1843200;}
		for(del=0; del<3; del++) {sceDisplayWaitVblankStart();}
		} // right
		if (pad.Buttons & PSP_CTRL_LEFT) {
		delay = delay - 1024;
		if (delay < 1) {delay = 1024;}
		for(del=0; del<3; del++) {sceDisplayWaitVblankStart();}
		} // left
		} // mode

		if (pad.Buttons & PSP_CTRL_CROSS) {
		sam = 1; // set sample address counter
		samlock = 1; // re-enable player routine
		samsel = 1; // select click sample
		if (!recording) {
		sceRtcGetCurrentClockLocalTime(&rtime);	// get the time from PSP real time clock
		fsec = rtime.seconds;
		fmn = rtime.minutes;
		fhour = rtime.hour;
		fyear = rtime.year;
		fmonth = rtime.month;
		fday = rtime.day;
		if (fyear > 1999) {			// convert year to a two digit integer
		if (fyear < 2100) {fyear = fyear - 2000;}
		}
		if (fyear > 2099) {
		if (fyear < 2200) {fyear = fyear - 2100;}
		}

		if (mictype < 4 || mictype == 7 || mictype == 8) {
		sprintf(recstate, "RECORDING");
		} else {
		sprintf(recstate, "RUNNING");
		} // mictype
		sceKernelSignalSema(startrecord, 1);
		sceKernelDelayThread(500000);
		recording = 1;
		hours = 0; minutes = 0; seconds = 0;
		} else {
		stoprecord = 1;
		sceKernelWaitSema(waitrecord, 1, NULL);
		recording = 0;
		sprintf(recstate, "STOPPED");
		if (mictype > 8 || mode == 1) {
		for(del=0; del<5; del++) {sceDisplayWaitVblankStart();}
		} // mictype
		for(del=0; del<4096; del++) {buffer[del] = 0;} // clear audio buffer
		for(del=0; del<1024; del++) {
		loop_buffer[del].l = 0; loop_buffer[del].r = 0;
		dbuf[del] = 0; ubuf[del] = 0;
		} // del

		}
		}

		if (pad.Buttons & PSP_CTRL_START) {
		if (recording == 1) {
		stoprecord = 1;
		sceKernelWaitSema(waitrecord, 1, NULL);
		recording = 0;
		} // recording
		for(del=0; del<5; del++) {sceDisplayWaitVblankStart();} // pause
		reset = 1;
		sceKernelSignalSema(startrecord, 1);
		reset = 0;
		sam = 1; // set sample address counter
		samlock = 1; // re-enable player routine
		samsel = 1; // select click sample
		reset_mode();
		} // start

		if (pad.Buttons & PSP_CTRL_TRIANGLE) {
		if (recording == 1) {
		stoprecord = 1;
		sceKernelWaitSema(waitrecord, 1, NULL);
		recording = 0;
		} // recording
		for(del=0; del<5; del++) {sceDisplayWaitVblankStart();} // pause
		reset = 1;
		sceKernelSignalSema(startrecord, 1);
		reset = 0;
		sam = 1; // set sample address counter
		samlock = 1; // re-enable player routine
		samsel = 1; // select click sample
		reset_mode();
		} // triangle

		oldpad.Buttons = pad.Buttons;
		sceKernelDelayThread(50000);
		}
		return 0;
}

void border() {
		drawLineScreen(5, 5, 120, 5, 0xFF555555);
		drawLineScreen(475, 5, 360, 5, 0xFF555555);
		drawLineScreen(5, 267, 475, 267, 0xFF555555);
		drawLineScreen(5, 5, 5, 267, 0xFF555555);
		drawLineScreen(475, 5, 475, 267, 0xFF555555);
}

void drawwave() {
		int i;
		int k;
		g = 1;				// main wave
		for (i = 10; i < 470; i++) { 
		k = buffer[g];
		if (mictype == 3 || mictype == 4 || mictype > 5) {k = ubuf[g] >> 8;}
		if (mictype > 8) {k = loop_buffer[g].l >> 8;}
		if (mictype == 2) {k = (k << 2);} else {k = (k << 1);} // mictype
		k = k+k;
		if (k > 265) k = 0; 
		else if (k < 0) k = 0;
		k = 265 - k;
		if (k < 260) {
		if (k > 10) {
		fillScreenRect(0xFFE22B8A, i, k, 4, 3);
		if (k < sens) {trigger = 40;}
		}}
		g = g + 2; i = i + 4;
		}

		if (mictype > 8) {		// delay channel wave
		g = 1;
		for (i = 10; i < 470; i++) { 
		k = dbuf[g] >> 8; k = (k << 1);
		k = k+k;
		if (k > 265) k = 0; 
		else if (k < 0) k = 0;
		k = 265 - k;
		if (k < 260) {
		if (k > 10) {fillScreenRect(0xFF00D7FF, i, k, 4, 3);}
		}
		g = g + 2; i = i + 4;
		}
		} // mictype

		if (vox == 1) {
		if (trigger == 0) {
		for (i = 10; i < 470; i++) {fillScreenRect(0xFF2222B2, i, sens, 2, 2);i = i + 2;}
		} else {
		for (i = 10; i < 470; i++) {fillScreenRect(0xFF008000, i, sens, 2, 2);i = i + 2;}
		trigger--;
		} // trigger
		} // vox
}

void free_ms_memory() {
		int memd;
		sceIoDevctl("ms0:", 0x02425818, &pbuf, sizeof(pbuf), 0, 0);
		for (memd=0; memd<5; memd++);
		freemem = buf[2] * buf[3] * buf[4];
		freebytes = freemem;
		freemem = freemem/1024;
		freemem = freemem/1024;
		sprintf(fillerm, "Memory Stick has %d Mb Free",freemem);
}

void calcfree() {
		gettime();
		if (mictype < 9) {
		freemem = freebytes/1024;
		freemem = freemem/1024;
		freemb = freemem;
		sprintf(fillert,"%02d:%02d:%02d",hours,minutes,seconds);
		sprintf(fillerm,"%d Mb",freemem);
		} else {
		milisecs = delay / 44.1;
		sprintf(fillerm,"%d ms",milisecs);
		} // mictype
}

void gettime() {
		sceRtcGetCurrentClockLocalTime(&rtime);	// get the time from PSP real time clock
		secx = sec;
		sec = rtime.seconds;
		if (sec != secx) {seconds++;}
		if (seconds == 60) {minutes++; seconds = 0;}
		if (minutes == 60) {hours++; minutes = 0;}
}

void fileexist() {
		int dfdx;
		int fdc;
		int bg;
		int there;

		folderexists = 0;				// check target folder exists
		dfdx = sceIoDopen("ms0:/MUSIC");
		if(dfdx >= 0) {folderexists = 1;}
		if (folderexists == 0) {sceIoMkdir("ms0:/MUSIC", 0777);}

		folderexists = 0;				// check target folder exists
		dfdx = sceIoDopen("ms0:/MUSIC/Audio Mechanica");
		if(dfdx >= 0) {folderexists = 1;}
		if (folderexists == 0) {sceIoMkdir("ms0:/MUSIC/Audio Mechanica", 0777);}

		bg = sceIoOpen("./oskbg", PSP_O_RDONLY, 0);	// check existence osk background
		if(bg < 0) {there = 0;} else {there = 1;}
		sceIoClose(bg);
		if (there == 0) {saveImage("./oskbg",background->data,480,272,PSP_LINE_SIZE,0);}

		fdc = sceIoOpen("./HTML.PBP", PSP_O_RDONLY, 0);	// check existence of html exec
		if(fdc < 0) {there = 0;} else {there = 1;}
		sceIoClose(fdc);
		sceDisplayWaitVblankStart();

		if (there == 0) {
		fdc = sceIoOpen("./HTML.PBP", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
		sceIoWrite(fdc,hpbp,49648);
		sceIoClose(fdc);
		} // there
		sceDisplayWaitVblankStart();

		fdc = sceIoOpen("./ebootloader.prx", PSP_O_RDONLY, 0);	// check existence of loader
		if(fdc < 0) {there = 0;} else {there = 1;}
		sceIoClose(fdc);
		sceDisplayWaitVblankStart();

		if (there == 0) {
		fdc = sceIoOpen("./ebootloader.prx", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
		sceIoWrite(fdc,load,2158);
		sceIoClose(fdc);
		} else {
		mchange = 1;	// cause the scrolling intro message to be shown only the first launch.
		} // there
		sceDisplayWaitVblankStart();
}

void saveScreen() {
		int posX;
		int posY;
		Color pixel;
		Color* vram = getVramDisplayBuffer();
		Image* screenShot;
		screenShot = createImage(480,272);
		for (posY=0; posY<272; posY++) {
		for(posX=0; posX<480; posX++) {
		pixel = vram[PSP_LINE_SIZE * posY + posX];
		putPixelImage(pixel,posX,posY,screenShot);
		}
		}
		saveImage("ms0:/Snapshot.png",screenShot->data,480,272,PSP_LINE_SIZE,0);
		freeImage(screenShot);
}

u32 ramAvailableLineareMax () {
		u32 size, sizeblock; 
		u8 *ram; 
		size = 0; 
		sizeblock = RAM_BLOCK; 
		while (sizeblock) { 
		size += sizeblock; 
		ram = malloc(size); 
		if (!(ram)) { 
		size -= sizeblock; 
		sizeblock >>= 1;
		}
		else
		free(ram);
		} 
		return size; 
} 

void GetRAMFree() { 
		u8 **ram, **temp; 
		u32 size, count, x;  
		ram = NULL; 
		size = 0; 
		count = 0; 
		for (;;) { 
		if (!(count % 10)) { 
		temp = realloc(ram,sizeof(u8 *) * (count + 10)); 
		if (!(temp)) break; 
		ram = temp; 
		size += (sizeof(u8 *) * 10);
		} 
		x = ramAvailableLineareMax(); 
		if (!(x)) break; 
		ram[count] = malloc(x); 
		if (!(ram[count])) break; 
		size += x; 
		count++; 
		} 
		if (ram) { 
		for (x=0;x<count;x++) free(ram[x]); 
		free(ram);
		} 
		sprintf(fillerram, "Free System RAM: %d Bytes", size);
}

void errorscreen() {
		for(del=0; del<900; del++) {
		clearScreen(0);			// print error screen
		sprintf(filler, "AN ERROR HAS OCCURED!");
		guStart();
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 10, fillerx);
		intraFontSetStyle(ltn8, 1.0f,0xFF6666FF,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 100, filler);
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 130, fillere);
		sceGuFinish();
		sceGuSync(0,0);
		border();
		sceDisplayWaitVblankStart();
		flipScreen();
		} // pause screen
}

void fullscreen() {
		for(del=0; del<500; del++) {
		clearScreen(0);			// print error screen
		sprintf(filler, "Recording has stopped!");
		guStart();
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 10, fillerx);
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 100, filler);
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 130, fillere);
		sceGuFinish();
		sceGuSync(0,0);
		border();
		sceDisplayWaitVblankStart();
		flipScreen();
		} // pause screen
}

void ramscreen() {
		GetRAMFree();
		clearScreen(0);
		guStart();
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 10, fillerx);
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 140, fillerram);
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 258, fillerm);
		sceGuFinish();
		sceGuSync(0,0);
		border();
		sceDisplayWaitVblankStart();
		flipScreen();
		for(del=0; del<190; del++) {sceDisplayWaitVblankStart();} // pause
}

void reset_mode() {
		vox = 0; mode = 0; recording = 0;
		delay = 10240; sens = 100; prxflag = 1;

		if (mictype == 11) {mictype = 3;}
		if (mictype == 10) {mictype = 2;}
		if (mictype == 9) {mictype = 1;}
		if (mictype == 8) {mictype = 2;}
		if (mictype == 7) {mictype = 1;}
		if (mictype == 6) {mictype = 3;}
		if (mictype == 5) {mictype = 2;}
		if (mictype == 4) {mictype = 1;}

		for(del=0; del<4096; del++) {buffer[del] = 0;} // clear audio buffers
		for(del=0; del<1024; del++) {
		loop_buffer[del].l = 0; loop_buffer[del].r = 0;
		dbuf[del] = 0; ubuf [del] = 0;
		} // del
		menu = 1;

		clearScreen(0);			// display message screen
		guStart();
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 10, fillerx);
		intraFontSetStyle(ltn8, 1.0f,0xFFEFDDCA,0,INTRAFONT_ALIGN_CENTER);
		intraFontPrint(ltn8, 240, 140, "Returning to Mode Menu");
		sceGuFinish();
		sceGuSync(0,0);
		border();
		sceDisplayWaitVblankStart();
		flipScreen();
		for(del=0; del<30; del++) {sceDisplayWaitVblankStart();} // pause
}











