#include <pspsdk.h>
#include <pspkernel.h>
#include <string.h>


#define VERS 1
#define REVS 0


PSP_MODULE_INFO("cooleyesBridge", 0x1006, VERS, REVS);
PSP_MAIN_THREAD_ATTR(0);

int sceAudioSetFrequency(int frequency);
int sceAudioSetFrequency370(int frequency);
int sceAudioSetFrequency380(int frequency);
int sceAudioSetFrequency395(int frequency);
int sceAudioSetFrequency500(int frequency);
int sceAudioSetFrequency620(int frequency);
int sceAudioSetFrequency630(int frequency);
int sceAudioSetFrequency660(int frequency);

int sceMeBootStart(int mebooterType);
int sceMeBootStart370(int mebooterType);
int sceMeBootStart380(int mebooterType);
int sceMeBootStart395(int mebooterType);
int sceMeBootStart500(int mebooterType);
int sceMeBootStart620(int mebooterType);
int sceMeBootStart630(int mebooterType);
int sceMeBootStart660(int mebooterType);

int cooleyesAudioSetFrequency(int devkitVersion, int frequency) {
	u32 k1; 
   	k1 = pspSdkSetK1(0);
   	int ret; 
	if (devkitVersion < 0x03070000)
		ret = sceAudioSetFrequency(frequency);    //3.5x-3.60
	else if ( devkitVersion < 0x03080000 )
		ret = sceAudioSetFrequency370(frequency); //3.7x
	else if ( devkitVersion < 0x03090500 )
		ret = sceAudioSetFrequency380(frequency); //3.80-3.90
	else if ( devkitVersion < 0x05000000 )
		ret = sceAudioSetFrequency395(frequency); //3.95-4.01
	else if ( devkitVersion < 0x06020000 )
		ret = sceAudioSetFrequency500(frequency); //5.00-5.50
	else if ( devkitVersion < 0x06030000 )
		ret = sceAudioSetFrequency620(frequency); //6.20
	else if ( devkitVersion < 0x06060000 )
		ret = sceAudioSetFrequency630(frequency); //6.30-6.39
	else
		ret = sceAudioSetFrequency660(frequency); //6.60-6.61
	pspSdkSetK1(k1);
	return ret;
}

int cooleyesMeBootStart(int devkitVersion, int mebooterType) {
	u32 k1; 
   	k1 = pspSdkSetK1(0);
   	int ret; 
   	if (devkitVersion < 0x03070000)
		ret = sceMeBootStart(mebooterType);
	else if ( devkitVersion < 0x03080000 )
		ret = sceMeBootStart370(mebooterType);
	else if ( devkitVersion < 0x03090500 )
		ret = sceMeBootStart380(mebooterType);
	else if ( devkitVersion < 0x05000000 )
		ret = sceMeBootStart395(mebooterType);
	else if ( devkitVersion < 0x06020000 )
		ret = sceMeBootStart500(mebooterType);
	else if ( devkitVersion < 0x06030000 )
		ret = sceMeBootStart620(mebooterType);
	else if ( devkitVersion < 0x06060000 )
		ret = sceMeBootStart630(mebooterType);
	else
		ret = sceMeBootStart660(mebooterType);
	pspSdkSetK1(k1);
	return ret;
}

int module_start(SceSize args, void *argp){
	return 0;
}

int module_stop(){
	return 0;
}
