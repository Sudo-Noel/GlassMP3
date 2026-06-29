/*
This source was taken (and modified) from:
PMP Mod
Copyright (C) 2006 jonny

Homepage: http://jonny.leffe.dnsalias.com
E-mail:   jonny@leffe.dnsalias.com

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
"opendir" c code (~3 lines of perl -> ~100 lines of c :s)
*/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "opendir.h"
#include "fat.h"
#include "libminiconv.h"
#include <pspsysmem.h>

static int FATinit = 0;

void opendir_safe_constructor(struct opendir_struct *p){
	p->directory_entry = 0;
	p->number_of_directory_entries = 0;
}


void opendir_close(struct opendir_struct *p){
	if (p->directory_entry) free_64(p->directory_entry);
	opendir_safe_constructor(p);
}


char *opendir_open(struct opendir_struct *p, const char *directory, const char *directoryShort, char extFilter[][5], int extNumber, int includeDirs){
	opendir_safe_constructor(p);

	SceUID dir = sceIoDopen(directoryShort);
	if(dir < 0){
		return("opendir_open: sceIoDopen failed");
	}

	int count = 0;
	SceIoDirent dirent;
	memset(&dirent, 0, sizeof(SceIoDirent));
	while(sceIoDread(dir, &dirent) > 0){
		count++;
	}
	
	if (count == 0){
		sceIoDclose(dir);
		return(0);
	}

	p->directory_entry = malloc_64(sizeof(struct dirElement) * count);
	if (p->directory_entry == 0){
		opendir_close(p);
		sceIoDclose(dir);
		return("opendir_open: malloc_64 failed on directory_entry");
	}

	sceIoDclose(dir);
	dir = sceIoDopen(directoryShort);
	
	int i = 0;
	while(sceIoDread(dir, &dirent) > 0){
		if (dirent.d_name[0] == '.'){
			continue;
		}
		
		char fileName[264] = "";
		if (directoryShort[strlen(directoryShort)-1] != '/')
			snprintf(fileName, sizeof(fileName), "%s/%s", directoryShort, dirent.d_name);
		else
			snprintf(fileName, sizeof(fileName), "%s%s", directoryShort, dirent.d_name);
			
		SceIoStat stat;
		memset(&stat, 0, sizeof(SceIoStat));
		if(sceIoGetstat(fileName, &stat) < 0){
			continue;
		}

		if (FIO_S_ISREG(stat.st_mode)){
			int extOK = 0;
			char ext[5] = "";
			getExtension(dirent.d_name, ext, 4);
			int j = 0;
			for (j = 0; j < extNumber; j++){
				if (strcmp(ext, extFilter[j]) == 0){
					extOK = 1;
					break;
				}
			}
			if (extOK == 0){
				continue;
			}
		} else if (FIO_S_ISDIR(stat.st_mode)){
			if (includeDirs == 0){
				continue;
			}
		} else {
			continue;
		}

		strcpy(p->directory_entry[p->number_of_directory_entries].d_name, dirent.d_name);
		strcpy(p->directory_entry[p->number_of_directory_entries].longname, dirent.d_name);
		memcpy(&p->directory_entry[p->number_of_directory_entries].d_stat, &stat, sizeof(SceIoStat));
		p->number_of_directory_entries++;
	}

	sceIoDclose(dir);
	sortDirectory(p);
	return(0);
}


//Ordinamento dei file di una directory:
void sortDirectory(struct opendir_struct *directory){
	int n = directory->number_of_directory_entries;
	int i = 0;
    char comp1[268] = "";
    char comp2[268] = "";

	while (i < n){
        snprintf(comp1, sizeof(comp1), "%s-%s", FIO_S_ISDIR(directory->directory_entry[i-1].d_stat.st_mode)?"A":"Z", directory->directory_entry[i-1].longname);
        snprintf(comp2, sizeof(comp2), "%s-%s", FIO_S_ISDIR(directory->directory_entry[i].d_stat.st_mode)?"A":"Z", directory->directory_entry[i].longname);

		if (i == 0 || strcasecmp(comp1, comp2) <= 0) i++;
		else {struct dirElement tmp = directory->directory_entry[i]; directory->directory_entry[i] = directory->directory_entry[i-1]; directory->directory_entry[--i] = tmp;}
	}
}

//Extract extension from a filename:
void getExtension(char *fileName, char *extension, int extMaxLength){
    int i = 0;
    int j = 0;
    int count = 0;
    for (i = strlen(fileName) - 1; i >= 0; i--){
        if (fileName[i] == '.'){
            if (i == strlen(fileName) - 1)
                return;
            for (j = i+1; j < strlen(fileName); j++){
                extension[count++] = toupper(fileName[j]);
                if (count > extMaxLength)
                    return;
            }
            extension[count] = '\0';
            return;
        }
    }
}

//Get directory up one level:
int directoryUp(char *dirName)
{
	if (strcmp(dirName, "ms0:/")){
		//Cerco l'ultimo slash:
		int i = 0;
		for (i = strlen(dirName) - 1; i >= 0; i--){
			if (dirName[i] == '/' && i != strlen(dirName)){
				if (i > 4){
					dirName[i] = '\0';
				}else{
					dirName[i+1] = '\0';
				}
				break;
			}
		}
		return(0);
	}else{
		return(-1);
	}
}

//Prendo solo il nome del file/directory:
void getFileName(char *fileName, char *onlyName){
	int slash = 0;

	//Cerco l'ultimo slash:
	int i = 0;
	int len = strlen(fileName);
	for (i = len - 2; i >= 0; i--){
		if (fileName[i] == '/'){
			slash = i;
			break;
		}
	}
	if (slash)
		strcpy(onlyName, fileName + slash + 1);
	else
		strcpy(onlyName, fileName);
}

//Check if a file esists (return fileSize if exists, -1 if doesen't exists):
int fileExists(char *fileName){
    SceIoStat stbuf;
    if(sceIoGetstat(fileName, &stbuf) < 0)
        return -1;
    return stbuf.st_size;
}
