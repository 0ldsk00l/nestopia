/*
	NEStopia / Linux
	Port by R. Belmont
	
	settings.cpp - settings handler class
*/

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "settings.h"

using namespace LinuxNst;

#define READ_SETTING(x) fread(&x, sizeof(x), 1, f)
#define WRITE_SETTING(x) if (fwrite(&x, sizeof(x), 1, f) != 1) std::cout << "Error writing setting!\n"


static int rates[4] = { 11025, 22050, 44100, 48000 };

// constructor
Settings::Settings()
{
	FILE *f;
	unsigned int signature;
	char dirname[1024], *home;

	// default settings
	videomode = 0;
	fullscreen = 0;
	ntscmode = 0;
	scaler = 0;
	sprlimit = 1;
	controls = 0;
	stereo = 0;
	volume = 100;
	rate = 3;
	rendertype = 1;
	scaleamt = 1;
	configitem = 0;
	prefsys = 0;
	spatch = 1;
	exciter = 0;
	litesurr = 0;
	surmult = 40;
	sndapi = 0;
	fsnativeres = 1;
	tvaspect = 0;
	blendpix = 1;
	cornerround = 0;
	oscanmask = 0;

	// try the recommended place for settings
	home = getenv("HOME");
	snprintf(dirname, sizeof(dirname), "%s/.nestopia/nstsettings", home);

	f = fopen(dirname, "rb");
	if (f == NULL)
	{
		std::cout << "~/.nestopia/nstsettings not found, creating a new one.\n";
		return;
	}

	// verify the file's magic number & version
	READ_SETTING(signature);
	if (signature != 0xaa550003)
	{
		std::cout << "Settings file invalid, using defaults\n";
		fclose(f);
		return;
	}

	READ_SETTING(ntscmode);
	READ_SETTING(videomode);
	READ_SETTING(fullscreen);
	READ_SETTING(scaler);
	READ_SETTING(sprlimit);
	READ_SETTING(rendertype);
	READ_SETTING(scaleamt);
	READ_SETTING(prefsys);
	READ_SETTING(controls);
	READ_SETTING(stereo);
	READ_SETTING(sndapi);
	READ_SETTING(volume);
	READ_SETTING(rate);
	READ_SETTING(configitem);
	READ_SETTING(spatch);
	READ_SETTING(exciter);
	READ_SETTING(litesurr);
	READ_SETTING(surmult);
	READ_SETTING(fsnativeres);
	READ_SETTING(tvaspect);
	READ_SETTING(blendpix);
	READ_SETTING(cornerround);
	READ_SETTING(oscanmask);

	fclose(f);
}

// destructor
Settings::~Settings()
{
	FILE *f;
	unsigned int signature = 0xaa550003;	// magic number/version
	char dirname[1024], *home;

	// make sure the output directory exists
	home = getenv("HOME");
	snprintf(dirname, sizeof(dirname), "%s/.nestopia/", home);
	mkdir(dirname, 0700);

	snprintf(dirname, sizeof(dirname), "%s/.nestopia/nstsettings", home);
	f = fopen(dirname, "wb");
	if (f == NULL)
	{
		std::cout << "Unable to write ~/.nestopia/nstsettings\n";
		return;
	}

	WRITE_SETTING(signature);
	WRITE_SETTING(ntscmode);
	WRITE_SETTING(videomode);
	WRITE_SETTING(fullscreen);
	WRITE_SETTING(scaler);
	WRITE_SETTING(sprlimit);
	WRITE_SETTING(rendertype);
	WRITE_SETTING(scaleamt);
	WRITE_SETTING(prefsys);
	WRITE_SETTING(controls);
	WRITE_SETTING(stereo);
	WRITE_SETTING(sndapi);
	WRITE_SETTING(volume);
	WRITE_SETTING(rate);
	WRITE_SETTING(configitem);
	WRITE_SETTING(spatch);
	WRITE_SETTING(exciter);
	WRITE_SETTING(litesurr);
	WRITE_SETTING(surmult);
	WRITE_SETTING(fsnativeres);
	WRITE_SETTING(tvaspect);
	WRITE_SETTING(blendpix);
	WRITE_SETTING(cornerround);
	WRITE_SETTING(oscanmask);

	fclose(f);
}

void Settings::SetRate(int sr) 
{ 
	rate = sr;
}

int Settings::GetRate(void) 
{ 
	return rates[rate]; 
}

int Settings::GetRawRate(void) 
{ 
	return rate;
}

