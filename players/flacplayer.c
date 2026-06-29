//    LightMP3
//    Copyright (C) 2007 Sakya
//    sakya_tg@yahoo.it
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//    CREDITS:
//    All credits for this goes to JLF65
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <utime.h>

#include <FLAC/stream_decoder.h>
#include <FLAC/format.h>
#include <FLAC/metadata.h>
#include "player.h"
#include "flacplayer.h"
#include "../system/opendir.h"

/////////////////////////////////////////////////////////////////////////////////////////
//Globals
/////////////////////////////////////////////////////////////////////////////////////////
static int bufferThid = -1;
static int FLAC_audio_channel;
static char FLAC_fileName[264];
static SceUID FLAC_fd = -1;
static int FLAC_eos = 0;
static struct fileInfo FLAC_info;
static int isPlaying = 0;
static unsigned int FLAC_volume_boost = 0;
static int FLAC_playingSpeed = 0; // 0 = normal
static int FLAC_playingDelta = 0;
static int outputInProgress = 0;
static long suspendPosition = -1;
static long suspendIsPlaying = 0;
int FLAC_defaultCPUClock = 333;

static int kill_flac_thread;
static int bufferLow;
static int FLAC_mutex = -1;

#define MIX_BUF_SIZE 65000
static short FLAC_mixBuffer[256 * 1024]__attribute__ ((aligned(64)));

static long FLAC_tempmixleft = 0;
static long samples_played = 0;
static long FLAC_total_samples = 0;

static FLAC__StreamDecoder *decoder = 0;
static double FLAC_newFilePos = -1;

/////////////////////////////////////////////////////////////////////////////////////////
//dummy functions for FLAC metadata iterators
/////////////////////////////////////////////////////////////////////////////////////////
int chmod(const char *path, mode_t mode)
{
	return 0;
}

int chown(const char *path, uid_t owner, gid_t group)
{
	return 0;
}

int utime(const char *path, const struct utimbuf *times)
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
//FLAC callbacks
/////////////////////////////////////////////////////////////////////////////////////////
FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data)
{
   int i;

   (void)decoder, (void)client_data;

   if (kill_flac_thread)
      return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

   sceKernelWaitSema(FLAC_mutex, 1, 0);

   int shift = 0;
   if (frame->header.bits_per_sample == 24) shift = 8;
   else if (frame->header.bits_per_sample == 32) shift = 16;

   int step = 1;
   if (frame->header.sample_rate == 192000 || frame->header.sample_rate == 176400) step = 4;
   else if (frame->header.sample_rate == 96000 || frame->header.sample_rate == 88200) step = 2;

   int added = 0;
   /* copy decoded PCM samples to buffer */
   for (i=0; i<frame->header.blocksize; i += step)
   {
      int j = (FLAC_tempmixleft + added)<<1;
      FLAC_mixBuffer[j] = (short)(buffer[0][i] >> shift);
      if (frame->header.channels > 1)
          FLAC_mixBuffer[j + 1] = (short)(buffer[1][i] >> shift);
      else
          FLAC_mixBuffer[j + 1] = (short)(buffer[0][i] >> shift);
      added++;
   }
   FLAC_tempmixleft += added; // increment # samples reported in buffer

   int should_wait = (FLAC_tempmixleft > MIX_BUF_SIZE);
   sceKernelSignalSema(FLAC_mutex, 1);

   if (should_wait)
      sceKernelWaitSema(bufferLow, 1, 0); // wait for buffer to get low

   return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE; // keep going until buffer is full or get killed
}

void metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
   (void)decoder, (void)client_data;
   
   if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
        FLAC_info.fileType = FLAC_TYPE;
        FLAC_info.defaultCPUClock = FLAC_defaultCPUClock;
        FLAC_info.kbit = metadata->data.stream_info.sample_rate * metadata->data.stream_info.bits_per_sample * metadata->data.stream_info.channels / 1000;
        FLAC_info.instantBitrate = metadata->data.stream_info.sample_rate * metadata->data.stream_info.bits_per_sample * metadata->data.stream_info.channels;
        
        // Handle high sample rates by downsampling
        FLAC_info.hz = metadata->data.stream_info.sample_rate;
        if (FLAC_info.hz == 192000) FLAC_info.hz = 48000;
        else if (FLAC_info.hz == 176400) FLAC_info.hz = 44100;
        else if (FLAC_info.hz == 96000) FLAC_info.hz = 48000;
        else if (FLAC_info.hz == 88200) FLAC_info.hz = 44100;
        
        FLAC_info.length = (long)(metadata->data.stream_info.total_samples / metadata->data.stream_info.sample_rate);
        FLAC_total_samples = (long)metadata->data.stream_info.total_samples;
        FLAC_info.needsME = 0;
        
        if (metadata->data.stream_info.channels == 1)
            strcpy(FLAC_info.mode, "single channel");
        else if (metadata->data.stream_info.channels == 2)
            strcpy(FLAC_info.mode, "stereo");
        else
            strcpy(FLAC_info.mode, "multi channel");
        
        strcpy(FLAC_info.emphasis, "no");
        
        long secs = FLAC_info.length;
        int h = secs / 3600;
        int m = (secs - h * 3600) / 60;
        int s = secs - h * 3600 - m * 60;
        snprintf(FLAC_info.strLength, sizeof(FLAC_info.strLength), "%2.2i:%2.2i:%2.2i", h, m, s);
   }
}

void error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
   (void)decoder, (void)client_data;

   //printf(" Error: %s\n", FLAC__StreamDecoderErrorStatusString[status]);
}

static FLAC__StreamDecoderReadStatus read_callback(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data)
{
    SceUID fd = (SceUID)client_data;
    if(*bytes > 0) {
        int read_bytes = sceIoRead(fd, buffer, *bytes);
        if(read_bytes < 0)
            return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
        else if(read_bytes == 0)
            return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
        else {
            *bytes = read_bytes;
            return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
        }
    }
    return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
}

static FLAC__StreamDecoderSeekStatus seek_callback(const FLAC__StreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data)
{
    SceUID fd = (SceUID)client_data;
    if(sceIoLseek32(fd, absolute_byte_offset, PSP_SEEK_SET) >= 0)
        return FLAC__STREAM_DECODER_SEEK_STATUS_OK;
    else
        return FLAC__STREAM_DECODER_SEEK_STATUS_ERROR;
}

static FLAC__StreamDecoderTellStatus tell_callback(const FLAC__StreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data)
{
    SceUID fd = (SceUID)client_data;
    long pos = sceIoLseek32(fd, 0, PSP_SEEK_CUR);
    if(pos >= 0) {
        *absolute_byte_offset = pos;
        return FLAC__STREAM_DECODER_TELL_STATUS_OK;
    } else
        return FLAC__STREAM_DECODER_TELL_STATUS_ERROR;
}

static FLAC__StreamDecoderLengthStatus length_callback(const FLAC__StreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data)
{
    SceUID fd = (SceUID)client_data;
    long current = sceIoLseek32(fd, 0, PSP_SEEK_CUR);
    long end = sceIoLseek32(fd, 0, PSP_SEEK_END);
    sceIoLseek32(fd, current, PSP_SEEK_SET);
    if(end >= 0) {
        *stream_length = end;
        return FLAC__STREAM_DECODER_LENGTH_STATUS_OK;
    } else
        return FLAC__STREAM_DECODER_LENGTH_STATUS_ERROR;
}

static FLAC__bool eof_callback(const FLAC__StreamDecoder *decoder, void *client_data)
{
    SceUID fd = (SceUID)client_data;
    long current = sceIoLseek32(fd, 0, PSP_SEEK_CUR);
    long end = sceIoLseek32(fd, 0, PSP_SEEK_END);
    sceIoLseek32(fd, current, PSP_SEEK_SET);
    return (current >= end);
}


/////////////////////////////////////////////////////////////////////////////////////////
//FLAC thread
/////////////////////////////////////////////////////////////////////////////////////////

int flacThread(SceSize args, void *argp)
{
   FLAC_eos = 0;
   kill_flac_thread = 0;

   // The decoder is already created and initialized in FLAC_Load!
   if(decoder == NULL) {
       sceKernelExitThread(0);
       return 0;
   }

   FLAC_tempmixleft = 0;
   FLAC_eos = 0;
   sceKernelSignalSema(bufferLow, 1); // so it fills the buffer to start

   while (FLAC__stream_decoder_process_single(decoder) != false){
        if (FLAC_newFilePos >= 0)
        {
            if (!FLAC_newFilePos){
                FLAC__uint64 sample = 0;
                if (FLAC__stream_decoder_seek_absolute(decoder, sample)) {
                    sceKernelWaitSema(FLAC_mutex, 1, 0);
                    samples_played = 0;
                    FLAC_tempmixleft = 0; // clear buffer of stale samples
                    sceKernelSignalSema(FLAC_mutex, 1);
                }
                if (FLAC__stream_decoder_get_state(decoder) == FLAC__STREAM_DECODER_SEEK_ERROR)
                    FLAC__stream_decoder_flush(decoder);
            }
            FLAC_newFilePos = -1;
        }

        //Check for playing speed:
        if (FLAC_playingSpeed){
            FLAC__uint64 sample = (FLAC__uint64)(samples_played + PSP_NUM_AUDIO_SAMPLES + FLAC_playingDelta);
            if (sample > FLAC_total_samples)
                break;
            if (sample < 0)
                sample = 0;

            if (!FLAC__stream_decoder_seek_absolute(decoder, sample)) {
                FLAC_setPlayingSpeed(0);
            } else {
                sceKernelWaitSema(FLAC_mutex, 1, 0);
                samples_played = sample;
                FLAC_tempmixleft = 0; // clear buffer of stale samples
                sceKernelSignalSema(FLAC_mutex, 1);
            }
            if (FLAC__stream_decoder_get_state(decoder) == FLAC__STREAM_DECODER_SEEK_ERROR)
                FLAC__stream_decoder_flush(decoder);
        }

        if (FLAC__stream_decoder_get_state(decoder) == FLAC__STREAM_DECODER_END_OF_STREAM )
            break;
   }

   FLAC_eos = 1;

   // Decoder deletion is handled safely by FLAC_FreeTune() to prevent double-free

   sceKernelExitThread(0);
   return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
//Audio callback
/////////////////////////////////////////////////////////////////////////////////////////
static void audioCallback(void *_buf2, unsigned int numSamples, void *pdata){
    short *_buf = (short *)_buf2;

	if (isPlaying) {	// Playing , so mix up a buffer
        outputInProgress = 1;

		while (1) {
            sceKernelWaitSema(FLAC_mutex, 1, 0);
            long temp_left = FLAC_tempmixleft;
            sceKernelSignalSema(FLAC_mutex, 1);

            if (temp_left >= numSamples)
                break;

			sceKernelSignalSema(bufferLow, 1);
			sceKernelDelayThread(10); // allow buffer filling thread to run
			if (FLAC_eos || kill_flac_thread) {	//EOF or killed
                isPlaying = 0;
				outputInProgress = 0;
				break;
			}
		}

		if (isPlaying) {
            sceKernelWaitSema(FLAC_mutex, 1, 0);
            if (FLAC_tempmixleft >= numSamples) {	//  Buffer has enough, so copy across
                int count, count2;
                short *_buf2;
                for (count = 0; count < numSamples; count++) {
                    count2 = count + count;
                    _buf2 = _buf + count2;
                    //Volume boost:
                    if (FLAC_volume_boost){
                        *(_buf2) = volume_boost(&FLAC_mixBuffer[count2], &FLAC_volume_boost);
                        *(_buf2 + 1) = volume_boost(&FLAC_mixBuffer[count2 + 1], &FLAC_volume_boost);
                    }else{
                        *(_buf2) = FLAC_mixBuffer[count2];
                        *(_buf2 + 1) = FLAC_mixBuffer[count2 + 1];
                    }
                }

                //  Move the pointers
                FLAC_tempmixleft -= numSamples;
                //  Now shuffle the buffer along
                memmove(FLAC_mixBuffer, &FLAC_mixBuffer[numSamples<<1], FLAC_tempmixleft * 2 * sizeof(short));
            }
            sceKernelSignalSema(FLAC_mutex, 1);
            samples_played += numSamples;
        } else {
            int count;
            for (count = 0; count < numSamples * 2; count++)
                *(_buf + count) = 0;
        }
    } else {			//  Not Playing , so clear buffer
        int count;
        for (count = 0; count < numSamples * 2; count++)
            *(_buf + count) = 0;
	}
    outputInProgress = 0;
}


/////////////////////////////////////////////////////////////////////////////////////////
//Support functions
/////////////////////////////////////////////////////////////////////////////////////////
static void splitComment(char *comment, char *name, int name_len, char *value, int value_len){
	char *result;
	int count = 0;
    
    name[0] = 0; value[0] = 0;

	result = strtok(comment, "=");
	while(result != NULL){
		if (count == 0){
			strncpy(name, result, name_len - 1);
            name[name_len - 1] = '\0';
		}else{
			if (count == 1) {
				strncpy(value, result, value_len - 1);
                value[value_len - 1] = '\0';
            }else{
                if (strlen(value) + 1 < value_len)
				    strcat(value, "=");
                int remaining = value_len - strlen(value) - 1;
                if (remaining > 0)
				    strncat(value, result, remaining);
			}
		}
		count++;
		result = strtok(NULL, "=");
	}
}


void getFLACTagInfo(char *filename, struct fileInfo *targetInfo){
	int i;
	char name[31];
	char value[264];
	FLAC__StreamMetadata *info = 0;

    strncpy(FLAC_fileName, filename, sizeof(FLAC_fileName) - 1);
    FLAC_fileName[sizeof(FLAC_fileName) - 1] = '\0';
	if (FLAC__metadata_get_tags(filename, &info)) {
		if(info->type == FLAC__METADATA_TYPE_VORBIS_COMMENT) {
			for(i = 0; i < info->data.vorbis_comment.num_comments; ++i) {
				splitComment((char*)info->data.vorbis_comment.comments[i].entry, name, sizeof(name), value, sizeof(value));
				if (!strcasecmp(name, "TITLE"))
					strncpy(targetInfo->title, value, sizeof(targetInfo->title)-1);
				else if(!strcasecmp(name, "ALBUM"))
					strncpy(targetInfo->album, value, sizeof(targetInfo->album)-1);
				else if(!strcasecmp(name, "ARTIST"))
					strncpy(targetInfo->artist, value, sizeof(targetInfo->artist)-1);
				else if(!strcasecmp(name, "GENRE"))
					strncpy(targetInfo->genre, value, sizeof(targetInfo->genre)-1);
				else if(!strcasecmp(name, "DATE") || !strcasecmp(name, "YEAR")){
                    strncpy(targetInfo->year, value, 4);
                    targetInfo->year[4] = '\0';
				}else if(!strcasecmp(name, "TRACKNUMBER"))
		            strncpy(targetInfo->trackNumber, value, sizeof(targetInfo->trackNumber)-1);
        		else if(!strcasecmp(name, "COVERART_UUENCODED")){
                    //COVER ART
                }
			}
		}
		FLAC__metadata_object_delete(info);
	}
    if (!strlen(targetInfo->title))
        getFileName(FLAC_fileName, targetInfo->title);
}




void FLAC_Init(int channel){
    initAudioLib();
    MIN_PLAYING_SPEED=-119;
    MAX_PLAYING_SPEED=119;
	FLAC_audio_channel = channel;
	samples_played = 0;
    FLAC_total_samples = 0;
    bufferLow = sceKernelCreateSema("bufferLow", 0, 1, 1, 0);
    FLAC_mutex = sceKernelCreateSema("FLAC_mutex", 0, 1, 1, 0);
    memset(FLAC_mixBuffer, 0, sizeof(FLAC_mixBuffer));
    FLAC_tempmixleft = 0;
    pspAudioSetChannelCallback(FLAC_audio_channel, audioCallback, NULL);
}


int FLAC_Load(char *filename){
	samples_played = 0;
	outputInProgress = 0;
	isPlaying = 0;
	FLAC_eos = 0;
    FLAC_playingSpeed = 0;
    kill_flac_thread = 0;
    initFileInfo(&FLAC_info);
	strncpy(FLAC_fileName, filename, sizeof(FLAC_fileName) - 1);
    FLAC_fileName[sizeof(FLAC_fileName) - 1] = '\0';

    FLAC_fd = sceIoOpen(FLAC_fileName, PSP_O_RDONLY, 0777);
	if (FLAC_fd >= 0) {
        FLAC_info.fileSize = sceIoLseek(FLAC_fd, 0, PSP_SEEK_END);
        sceIoLseek(FLAC_fd, 0, PSP_SEEK_SET);
	}else{
		return ERROR_OPENING;
	}
    
    // Create decoder to get metadata synchronously before thread starts
    if((decoder = FLAC__stream_decoder_new()) == NULL) {
        sceIoClose(FLAC_fd);
        FLAC_fd = -1;
        return ERROR_OPENING;
    }

    FLAC__StreamDecoderInitStatus init_status = FLAC__stream_decoder_init_stream(
        decoder,
        read_callback, seek_callback, tell_callback, length_callback, eof_callback,
        write_callback, metadata_callback, error_callback,
        (void*)FLAC_fd
    );

    if(init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        FLAC__stream_decoder_delete(decoder);
        decoder = NULL;
        sceIoClose(FLAC_fd);
        FLAC_fd = -1;
        return ERROR_OPENING;
    }

    FLAC__stream_decoder_process_until_end_of_metadata(decoder);

    getFLACTagInfo(FLAC_fileName, &FLAC_info);

    // Now FLAC_info.hz is populated synchronously by libFLAC
    if (FLAC_info.hz == 0 || (FLAC_info.hz != 8000 && FLAC_info.hz != 11025 && 
        FLAC_info.hz != 12000 && FLAC_info.hz != 16000 && FLAC_info.hz != 22050 && 
        FLAC_info.hz != 24000 && FLAC_info.hz != 32000 && FLAC_info.hz != 44100 && 
        FLAC_info.hz != 48000)) {
        FLAC_info.hz = 44100; // Fallback to avoid error -2
    }

    if (pspAudioSetFrequency(FLAC_info.hz) < 0){
        FLAC_FreeTune();
        return ERROR_INVALID_SAMPLE_RATE;
    }

	//Start buffer filling thread:
    bufferThid = -1;
	bufferThid = sceKernelCreateThread("bufferFilling", flacThread, 0x11, DEFAULT_THREAD_STACK_SIZE, PSP_THREAD_ATTR_USER, NULL);
	if(bufferThid < 0) {
        FLAC_FreeTune();
		return ERROR_CREATE_THREAD;
    }
	sceKernelStartThread(bufferThid, 0, NULL);

	return OPENING_OK;
}


int FLAC_Play(){
	isPlaying = 1;
	return 0;
}


void FLAC_Pause(){
	isPlaying = !isPlaying;
}


int FLAC_Stop(){
	isPlaying = 0;
	return 0;
}


void FLAC_FreeTune(){
	kill_flac_thread = 1;
	sceKernelSignalSema(bufferLow, 1);
    while (outputInProgress == 1)
        sceKernelDelayThread(100000);
    if (bufferThid >= 0) {
        sceKernelWaitThreadEnd(bufferThid, NULL);
        sceKernelDeleteThread(bufferThid);
        bufferThid = -1;
    }

	sceKernelDelayThread(100*1000);
	sceKernelDeleteSema(bufferLow);
    sceKernelDeleteSema(FLAC_mutex);
    if (decoder) {
        // FLAC__stream_decoder_delete calls finish, which might try to read/flush. Since we kill it abruptly, it's safer to just delete it.
        FLAC__stream_decoder_delete(decoder);
        decoder = NULL;
    }
    if (FLAC_fd >= 0) {
        sceIoClose(FLAC_fd);
        FLAC_fd = -1;
    }
    memset(FLAC_mixBuffer, 0, sizeof(FLAC_mixBuffer));
    FLAC_tempmixleft = 0;
}


void FLAC_GetTimeString(char *dest){
	char timeString[9];
	long secs = 0;
	if (FLAC_info.hz)
    	secs = samples_played / FLAC_info.hz;
	int h = secs / 3600;
	int m = (secs - h * 3600) / 60;
	int s = secs - h * 3600 - m * 60;
	snprintf(timeString, sizeof(timeString), "%2.2i:%2.2i:%2.2i", h, m, s);
	strcpy(dest, timeString);
}


int FLAC_EndOfStream(){
	return FLAC_eos;
}


struct fileInfo *FLAC_GetInfo(){
	return &FLAC_info;
}


struct fileInfo FLAC_GetTagInfoOnly(char *filename){
    struct fileInfo tempInfo;
    initFileInfo(&tempInfo);
    getFLACTagInfo(filename, &tempInfo);
    return tempInfo;
}


float FLAC_GetPercentage(){
    float perc = ((float)samples_played / (float)FLAC_info.hz) / (float)FLAC_info.length * 100.0;
    if (perc > 100)
        perc = 100;
	return perc;
}


void FLAC_End(){
    FLAC_Stop();
	pspAudioSetChannelCallback(FLAC_audio_channel, 0, 0);
	FLAC_FreeTune();
	endAudioLib();
}


int FLAC_setMute(int onOff){
    return setMute(FLAC_audio_channel, onOff);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Fade out:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FLAC_fadeOut(float seconds){
    fadeOut(FLAC_audio_channel, seconds);
}


void FLAC_setVolumeBoost(int boost){
    FLAC_volume_boost = boost;
}

int FLAC_getVolumeBoost(){
	return FLAC_volume_boost;
}


int FLAC_setPlayingSpeed(int playingSpeed){
	if (playingSpeed >= MIN_PLAYING_SPEED && playingSpeed <= MAX_PLAYING_SPEED){
		if (playingSpeed == 0)
			setVolume(FLAC_audio_channel, 0x8000);
		else
			setVolume(FLAC_audio_channel, FASTFORWARD_VOLUME);
        FLAC_playingDelta = PSP_NUM_AUDIO_SAMPLES * playingSpeed;
		FLAC_playingSpeed = playingSpeed;
		return 0;
	}else{
		return -1;
	}
}

int FLAC_getPlayingSpeed(){
	return FLAC_playingSpeed;
}

int FLAC_GetStatus(){
	return 0;
}

void FLAC_setVolumeBoostType(char *boostType){
    //Only old method supported
    MAX_VOLUME_BOOST = 4;
    MIN_VOLUME_BOOST = 0;
}


//Functions for filter (equalizer):
int FLAC_setFilter(double tFilter[32], int copyFilter){
	return 0;
}

void FLAC_enableFilter(){}

void FLAC_disableFilter(){}

int FLAC_isFilterSupported(){
	return 0;
}

int FLAC_isFilterEnabled(){
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Manage suspend:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int FLAC_suspend(){
    suspendPosition = samples_played;
    suspendIsPlaying = isPlaying;
    FLAC_End();
    return 0;
}

int FLAC_resume(){
    if (suspendPosition >= 0){
	   FLAC__uint64 sample = (FLAC__uint64)suspendPosition;
       FLAC_Load(FLAC_fileName);
       if (FLAC__stream_decoder_seek_absolute(decoder, sample)){
		  if (FLAC__stream_decoder_get_state(decoder) == FLAC__STREAM_DECODER_SEEK_ERROR)
			  FLAC__stream_decoder_flush(decoder);
          samples_played = suspendPosition;
          if (suspendIsPlaying)
             FLAC_Play();
       }
       suspendPosition = -1;
    }
    return 0;
}

double FLAC_getFilePosition()
{
    FLAC__uint64 pos = 0;
    FLAC__stream_decoder_get_decode_position(decoder, &pos);
    return (double)pos;
}

void FLAC_setFilePosition(double position)
{
    FLAC_newFilePos = position;
}
