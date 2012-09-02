#ifndef _SETTINGS_H_
#define _SETTINGS_H_

namespace LinuxNst
{
	class Settings
	{
		public:
			Settings();
			~Settings();

			// video accessors
			int GetFullscreen(void) { return fullscreen; }
			int GetScale(void) { return scaler; }
			int GetSprlimit(void) { return sprlimit; }
			int GetVideoMode(void) { return videomode; }
			int GetNtscMode(void) { return ntscmode; }
			int GetRenderType(void) { return rendertype; }
			int GetScaleAmt(void) { return scaleamt; }
			void SetFullscreen(int fs) { fullscreen = fs; }
			void SetScale(int scl) { scaler = scl; }
			void SetSprlimit(int limit) { sprlimit = limit; }
			void SetVideoMode(int mode) { videomode = mode; }
			void SetNtscMode(int mode) { ntscmode = mode; }
			void SetRenderType(int mode) { rendertype = mode; }
			void SetScaleAmt(int mode) { scaleamt = mode; }

			// input accessors
			int GetUseJoypads(void) { return controls; }
			void SetUseJoypads(int usejp) { controls = usejp; }
			int GetConfigItem(void) { return configitem; }
			void SetConfigItem(int item) { configitem = item; }

			// sound accessors
			int GetStereo(void) { return stereo; }
			int GetSndAPI(void) { return sndapi; }
			int GetVolume(void) { return volume; };
			int GetRate(void);
			int GetRawRate(void);
			int GetUseExciter(void) { return exciter; }
			int GetUseSurround(void) { return litesurr; }
			int GetSurrMult(void) { return surmult; }
			void SetStereo(int st) { stereo = st; }
			void SetSndAPI(int api) { sndapi = api; }
			void SetVolume(int vol) { volume = vol; }
			void SetRate(int sr);
			void SetUseExciter(int val) { exciter = val; }
			void SetUseSurround(int val) { litesurr = val; }
			void SetSurrMult(int val) { surmult = val; }

			// other accessors
			int GetPrefSystem(void) { return prefsys; }
			void SetPrefSystem(int mode) { prefsys = mode; }
			int GetSoftPatch(void) { return spatch; }
			void SetSoftPatch(int mode) { spatch = mode; }

		private:
			// video settings
			int fullscreen, scaler, sprlimit, videomode, ntscmode;
			int rendertype, scaleamt;
			// input settings
			int controls, configitem;
			// sound settings
			int stereo, sndapi, volume, rate, exciter, litesurr, surmult;
			// other settings
			int prefsys, spatch;
};
};

#endif

