/*
	NEStopia / Linux
	Port by R. Belmont
	
	seffect.cpp - DSP sound effects by Duddie
*/

#include <math.h>
#include <stdlib.h>

#include "settings.h"

#define EXCITER_MAXGAIN			(256*6)
#define EXCITER_SAFETHRESHOLD		(0x6000)
#define EXCITER_COMPTHRESHOLD		(0x7f00)

static LinuxNst::Settings *sSettings;

static float	seffect_ex_nowgain;
static float	seffect_ex_attgainunit;
static float	seffect_ex_boostgainunit;
static int	seffect_ex_boostcnt;
static int	seffect_ex_wkcnt;

// "lite surround" processor
int seffect_surround_lite_process(short *data, int length)
{
	int i;
	double avg, ldiff, rdiff, tmp;
	double mul;
	int seffect_surround_lite_multiplier = sSettings->GetSurrMult();

	mul = (double)seffect_surround_lite_multiplier / 10.0f;

	for (i = 0; i < length / 2; i += 2)
	{
		avg = (data[i] + data[i + 1]) / 2;
		ldiff = data[i] - avg;
		rdiff = data[i + 1] - avg;

		tmp = avg + ldiff * mul;
		if (tmp < -32768)
			tmp = -32768;
		if (tmp > 32767)
			tmp = 32767;
		data[i] = (short)tmp;

		tmp = avg + rdiff * mul;
		if (tmp < -32768)
			tmp = -32768;
		if (tmp > 32767)
			tmp = 32767;
		data[i + 1] = (short)tmp;
	}
	return length;
}

// stereo exciter process
void seffect_ex_process(long *pBuf,long samples)
{
	int	i;
	int	dt[2],absdt[2],maxdt;
	int	igain;
	
	for(i=0;i<samples;i++)
	{
		igain=(long)seffect_ex_nowgain;
		dt[0]=((pBuf[0])*igain)>>8;	absdt[0]=abs(dt[0]);
		dt[1]=((pBuf[1])*igain)>>8;	absdt[1]=abs(dt[1]);
		if(absdt[0] > absdt[1])	maxdt=absdt[0];
		else					maxdt=absdt[1];
		//
		if(maxdt > EXCITER_COMPTHRESHOLD)
		{	//do attenate
			seffect_ex_nowgain -= seffect_ex_attgainunit;
			if( seffect_ex_nowgain < 0 )
					seffect_ex_nowgain = 0;
			/*
			nowGain = (AUTOLEVEL_COMPTHRESHOLD/(double)maxdt)*nowGain;
			*/
		}
		else if((maxdt < EXCITER_SAFETHRESHOLD))
		{
			if(seffect_ex_wkcnt > seffect_ex_boostcnt)
			{
				if( maxdt > 0 )
					 seffect_ex_nowgain += seffect_ex_boostgainunit;
				if(seffect_ex_nowgain > EXCITER_MAXGAIN)
					seffect_ex_nowgain = EXCITER_MAXGAIN;
			}
			seffect_ex_wkcnt++;
			if(seffect_ex_wkcnt > 0x7fff0000)	
				seffect_ex_wkcnt = 0x7fff0000;
		}
		else
		{
			seffect_ex_wkcnt=0;
		}
		pBuf[0]=dt[0];
		pBuf[1]=dt[1];
		pBuf+=2;
	}
}

// stereo exciter init
void seffect_init(LinuxNst::Settings *settings)
{
	// get our pointer to the settings object
	sSettings = settings;

	// sound exciter init
	seffect_ex_wkcnt         = 0;
	seffect_ex_nowgain       = 256;
	seffect_ex_attgainunit   = (float)((44100.0/(double)sSettings->GetRate()) * 0.2);
	seffect_ex_boostgainunit = (float)((44100.0/(double)sSettings->GetRate()) * (100.0 / (0.2 * 44100.0)));
	seffect_ex_boostcnt      = sSettings->GetRate() / 6;
}
