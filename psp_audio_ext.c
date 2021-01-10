/**
 * 		PSP Audio Input Library v0.35
 * 
 * 		Copyright (c) 2006 Luis Teixeira <teixeluis@gmail.com>
 * 
 * 		Last Modified 28/08/2006
 * 	
 *		Defines functions for simplifying audio input usage within applications.
 * 
 */

#include "psp_audio_ext.h"

#include <pspdebug.h>
#include <stdlib.h>
#include <string.h>
#include <pspthreadman.h>

#include "logging.h"

#define printf	pspDebugScreenPrintf


// buffer_size = num_audio_samples * num_buffers

audio_input_buffer_t input_buf;

psp_audio_input_info AudioInputStatus;

int audio_input_ready = 0;
int audio_input_terminate = 0;

int appCallbackThread(SceSize args, void *argp) {
	printLog("appCallbackThread: entered thread.\n");
	void* bufptr;
	
	while(audio_input_terminate == 0) {
		bufptr = (void*) &(input_buf.buffer[input_buf.read_index * FRAME_SIZE]);
		pspAudioCallback_t callback;
		callback = AudioInputStatus.callback;

		if(input_buf.used_frames > 0) {
			if (callback)
				callback(bufptr, FRAME_SIZE, AudioInputStatus.pdata);		
	
			if(input_buf.read_index < NUM_FRAMES - 1)
				input_buf.read_index++;
			else 
				input_buf.read_index = 0;
			input_buf.used_frames--;
		}
			
		sceKernelDelayThread(0);
	}
	sceKernelExitThread(0);
	return 0;	
}

int audioInputThread(SceSize args, void *argp) {
	printLog("audioInputThread: entered thread.\n");
	void* bufptr;	

	while(audio_input_terminate == 0) {
		bufptr = (void*) &(input_buf.buffer[input_buf.write_index * FRAME_SIZE]);
		
		if(input_buf.used_frames < NUM_FRAMES) {
			// capture PSP_NUM_AUDIO_SAMPLES at the configured sample rate:
			pspAudioInputBlocking(FRAME_SIZE, AudioInputStatus.sample_rate, bufptr);
			
			if(input_buf.write_index < NUM_FRAMES - 1)
				input_buf.write_index++;
			else 
				input_buf.write_index = 0;

			input_buf.used_frames++;
		}
		sceKernelDelayThread(0);
	}
	sceKernelExitThread(0);
	return 0;
}

/******************************************************************************/


int pspAudioInputInit(int volume, int sample_rate) {
	int cb_th_ret = 0;
	int audio_in_th_ret = 0;	
	int failed = 0;
	
	audio_input_terminate = 1;
	audio_input_ready = 0;

    AudioInputStatus.handle = -1;
    AudioInputStatus.audio_in_th_handle = -1;
    AudioInputStatus.cb_th_handle = -1;
    AudioInputStatus.volume = volume;
    AudioInputStatus.sample_rate = sample_rate;
    AudioInputStatus.callback = 0;
    AudioInputStatus.pdata = 0;
	
	input_buf.read_index = 0;
	input_buf.write_index = 0;
	input_buf.used_frames = 0;

	memset(input_buf.buffer, 0, sizeof(short) * FRAME_SIZE * NUM_FRAMES);
	
	if((AudioInputStatus.handle = sceAudioInputInit(0, AudioInputStatus.volume, 0)) < 0) {
	   	printLog("pspAudioInputInit: AudioInputStatus.handle = %d\n", AudioInputStatus.handle);
		printLog("pspAudioInputInit: initialization failed.\n");

		AudioInputStatus.handle = -1;

		return -1;
	}
	
	audio_input_ready = 1;
	audio_input_terminate = 0;
	sceKernelDelayThread(600000);
	
	printLog("pspAudioInputInit: creating app_callback_thread.\n");
	//AudioInputStatus.cb_th_handle = sceKernelCreateThread("app_callback_thread", (void*) &appCallbackThread, 0x11, 0xFA0, 0, 0);
	AudioInputStatus.cb_th_handle = sceKernelCreateThread("app_callback_thread", (void*) &appCallbackThread, 0x12, 0xFA0, 0, NULL);
	printLog("pspAudioInputInit: app_callback_thread creation successful. threadhandle = %d\n", AudioInputStatus.cb_th_handle);

	if(AudioInputStatus.cb_th_handle < 0) {
		AudioInputStatus.cb_th_handle = -1;
		failed = 1;
	} 
	else {
		printLog("pspAudioInputInit: starting app_callback_thread.\n");
		cb_th_ret = sceKernelStartThread(AudioInputStatus.cb_th_handle, 0, 0);
		printLog("pspAudioInputInit: app_callback_thread successfully started.\n");
		printLog("pspAudioInputInit: sceKernelStartThread app_callback_thread return value = %d\n", cb_th_ret);
	}
		
	printLog("pspAudioInputInit: creating audio_input_thread.\n");
	//AudioInputStatus.audio_in_th_handle = sceKernelCreateThread("audio_input_thread", (void*) &audioInputThread, 0x11, 0xFA0, 0, 0);
	AudioInputStatus.audio_in_th_handle = sceKernelCreateThread("audio_input_thread", (void*) &audioInputThread, 0x12, 0xFA0, 0, NULL);
	printLog("pspAudioInputInit: audio_input_thread creation successful. threadhandle = %d\n", AudioInputStatus.audio_in_th_handle);
	
	if(AudioInputStatus.audio_in_th_handle < 0) {
		AudioInputStatus.audio_in_th_handle = -1;
		failed = 1;
	}
	else {
		printLog("pspAudioInputInit: starting audio_input_thread.\n");		
		audio_in_th_ret = sceKernelStartThread(AudioInputStatus.audio_in_th_handle, 0, 0);
		printLog("pspAudioInputInit: audio_input_thread successfully started.\n");		
		printLog("pspAudioInputInit: sceKernelStartThread audio_input_thread return value = %d\n", audio_in_th_ret);		
	}
		
	if(failed) {
		audio_input_terminate = 1;

		if(AudioInputStatus.cb_th_handle != -1) {
			printLog("pspAudioInputInit: going to delete app_callback_thread.\n");
			sceKernelDeleteThread(AudioInputStatus.cb_th_handle);			
			printLog("pspAudioInputInit: app_callback_thread thread successfully deleted.\n");
		}
		
		if(AudioInputStatus.audio_in_th_handle != -1) {
			printLog("pspAudioInputInit: going to delete audio_input_thread.\n");
			sceKernelDeleteThread(AudioInputStatus.audio_in_th_handle);
			printLog("pspAudioInputInit: audio_input_thread successfully deleted.\n");			
		}
		
		AudioInputStatus.audio_in_th_handle = -1;
		AudioInputStatus.cb_th_handle = -1;		
		audio_input_ready = 0;
		
		return -1;
	}

	return 0;	
}

int pspAudioInputSoftInit(int volume) {
	audio_input_ready = 0;

    AudioInputStatus.volume = volume;
        	
	if(AudioInputStatus.handle == -1 && (AudioInputStatus.handle = sceAudioInputInit(0, AudioInputStatus.volume, 0)) < 0)
		return -1;
	
	audio_input_ready = 1;
	
	return 0;
}

void pspAudioInputSoftEnd() {
	audio_input_ready = 0;

	AudioInputStatus.handle = -1;
}

void pspAudioInputEnd() {
	printLog("pspAudioInputEnd: function invoked.\n");

	audio_input_ready = 0;
	audio_input_terminate = 1;
	
	if(AudioInputStatus.audio_in_th_handle != -1 && AudioInputStatus.cb_th_handle != -1) {
		printLog("pspAudioInputEnd: going to delete audio input handling threads.\n");
		sceKernelTerminateDeleteThread(AudioInputStatus.cb_th_handle);
		sceKernelTerminateDeleteThread(AudioInputStatus.audio_in_th_handle);
		printLog("pspAudioInputEnd: audio input handling threads successfully deleted.\n");
		
		sceKernelWaitThreadEnd(AudioInputStatus.cb_th_handle, NULL);
		sceKernelWaitThreadEnd(AudioInputStatus.audio_in_th_handle, NULL);
	}
	
    AudioInputStatus.handle = -1;
    AudioInputStatus.audio_in_th_handle = -1;
    AudioInputStatus.cb_th_handle = -1;
    AudioInputStatus.volume = 0;
    AudioInputStatus.sample_rate = 0;
    AudioInputStatus.callback = 0;
    AudioInputStatus.pdata = 0;
    
	input_buf.read_index = 0;
	input_buf.write_index = 0;
	input_buf.used_frames = 0;	
}

void pspAudioSetInputVolume(int vol) {
	AudioInputStatus.volume = vol;	
}

void pspAudioSetInputCallback(pspAudioCallback_t callback, void *pdata) {
	volatile psp_audio_input_info *pii = &AudioInputStatus;
	pii->pdata=pdata;
	pii->callback=callback;
	printLog("pspAudioSetInputCallback: callback = %d\n", (int) callback);
}

int pspAudioInputBlocking(s32 length, s32 sample_rate, void *buf) {
	if (!audio_input_ready)
		return -1;
		
    sceAudioInputBlocking(length, sample_rate, buf);
	
	return 0;
}

int pspAudioInput(s32 length, s32 sample_rate, void *buf) {
	if (!audio_input_ready)
		return -1;
		
    sceAudioInput(length, sample_rate, buf);
	
	return 0;
}

