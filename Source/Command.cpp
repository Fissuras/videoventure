#include "StdAfx.h"

#include "Command.h"
#include "Console.h"
#include "World.h"
#include "Sound.h"
#include "GameState.h"


// console
extern Console *console;

// open window
extern bool OpenWindow(void);

// close window
extern void CloseWindow(void);

// init input
extern bool InitInput(const char *config);

// split level
extern bool SplitLevel(const char *config, const char *output);

// merge level
extern bool MergeLevel(const char *config, const char *output);


// COMMAND INSTANCE

// get the command database
Database::Typed<Command::Entry> &Command::GetDB()
{
	static Database::Typed<Entry> onactivate;
	return onactivate;
}

// constructor
Command::Command(unsigned int aDatabaseId, Entry aEntry)
	: mTagId(aDatabaseId)
{
	Database::Typed<Entry> &db = GetDB();
	Entry &entry = db.Open(mTagId);
	mPrev = entry;
	entry = aEntry;
	db.Close(mTagId);
}

// destructor
Command::~Command()
{
	Database::Typed<Entry> &db = GetDB();
	if (mPrev)
		db.Put(mTagId, mPrev);
	else
		db.Delete(mTagId);
}

// get the command with the specified identifier
const Command::Entry &Command::Get(unsigned aTagId)
{
	return GetDB().Get(aTagId);
}


// post-command function
typedef void (*ProcessCommandPostFunc)(void);

// process a string command
int ProcessCommandString(std::string &aValue, const char * const aParam[], int aCount, ProcessCommandPostFunc aAction, const char *aFormat)
{
	if (aCount >= 1)
	{
		aValue = aParam[0];
		if (aAction)
			aAction();
		return 1;
	}
	else if (console)
	{
		console->Print(aFormat, aValue.c_str());
		return 0;
	}
	else
	{
		DebugPrint(aFormat, aValue.c_str());
		return 0;
	}
}

// process a boolean command
int ProcessCommandBool(bool &aValue, const char * const aParam[], int aCount, ProcessCommandPostFunc aAction, const char *aFormat)
{
	if (aCount >= 1)
	{
		aValue = atoi(aParam[0]) != 0;
		if (aAction)
			aAction();
		return 1;
	}
	else if (console)
	{
		console->Print(aFormat, aValue);
		return 0;
	}
	else
	{
		DebugPrint(aFormat, aValue);
		return 0;
	}
}

// process an integer command
int ProcessCommandInt(int &aValue, const char * const aParam[], int aCount, ProcessCommandPostFunc aAction, const char *aFormat)
{
	if (aCount >= 1)
	{
		aValue = atoi(aParam[0]);
		if (aAction)
			aAction();
		return 1;
	}
	else if (console)
	{
		console->Print(aFormat, aValue);
		return 0;
	}
	else
	{
		DebugPrint(aFormat, aValue);
		return 0;
	}
}

// process a two-integer command
int ProcessCommandInt2(int &aValue1, int &aValue2, const char * const aParam[], int aCount, ProcessCommandPostFunc aAction, const char *aFormat)
{
	if (aCount >= 2)
	{
		aValue1 = atoi(aParam[0]);
		aValue2 = atoi(aParam[1]);
		if (aAction)
			aAction();
		return 2;
	}
	else if (console)
	{
		console->Print(aFormat, aValue1, aValue2);
		return 0;
	}
	else
	{
		DebugPrint(aFormat, aValue1, aValue2);
		return 0;
	}

}

// process a float command
int ProcessCommandFloat(float &aValue, const char * const aParam[], int aCount, ProcessCommandPostFunc aAction, const char *aFormat)
{
	if (aCount >= 1)
	{
		aValue = float(atof(aParam[0]));
		if (aAction)
			aAction();
		return 1;
	}
	else if (console)
	{
		console->Print(aFormat, aValue);
		return 0;
	}
	else
	{
		DebugPrint(aFormat, aValue);
		return 0;
	}
}

void UpdateWindowAction()
{
	if (runtime)
	{
		CloseWindow();
		OpenWindow();
	}
}
void InitInputAction()
{
	if (runtime)
	{
		InitInput(INPUT_CONFIG.c_str());
	}
}
void InitLevelAction()
{
	if (runtime)
	{
		if (curgamestate == STATE_PLAY)
		{
			setgamestate = STATE_RELOAD;
		}
	}
}
void InitRecordAction()
{
	record = 1;
}
void InitPlaybackAction()
{
	playback = 1;
}
void ClampMotionBlurAction()
{
	if (MOTIONBLUR_STEPS < 1)
		MOTIONBLUR_STEPS = 1;
}

int CommandShell(const char * const aParam[], int aCount)
{
	setgamestate = STATE_SHELL;
	return 0;
}
Command commandshell(0x11e1fc01 /* "shell" */, CommandShell);

int CommandPlay(const char * const aParam[], int aCount)
{
	setgamestate = STATE_PLAY;
	return 0;
}
Command commandplay(0xc2cbd863 /* "play" */, CommandPlay);

int CommandReload(const char * const aParam[], int aCount)
{
	setgamestate = STATE_RELOAD;
	return 0;
}
Command commandreload(0xed7cdd8c /* "reload" */, CommandReload);

int CommandQuit(const char * const aParam[], int aCount)
{
	setgamestate = STATE_QUIT;
	return 0;
}
Command commandquit(0x47878736 /* "quit" */, CommandQuit);

int CommandResolution(const char * const aParam[], int aCount)
{
	return ProcessCommandInt2(SCREEN_WIDTH, SCREEN_HEIGHT, aParam, aCount, UpdateWindowAction, "resolution: %dx%d\n"); 
}
Command commandresolution(0x1d215c8f /* "resolution" */, CommandResolution);

int CommandDepth(const char * const aParam[], int aCount)
{
	return ProcessCommandInt(SCREEN_DEPTH, aParam, aCount, UpdateWindowAction, "depth: %dbpp");
}
Command commanddepth(0xfe759eea /* "depth" */, CommandDepth);

int CommandFullScreen(const char * const aParam[], int aCount)
{
	return ProcessCommandBool(SCREEN_FULLSCREEN, aParam, aCount, UpdateWindowAction, "fullscreen: %d\n");
}
Command commandfullscreen(0x5032fb58 /* "fullscreen" */, CommandFullScreen);

int CommandVsync(const char * const aParam[], int aCount)
{
	return ProcessCommandBool(OPENGL_SWAPCONTROL, aParam, aCount, UpdateWindowAction, "vsync: %d\n");
}
Command commandvsync(0x06f8f066 /* "vsync" */, CommandVsync);

int CommandMultisample(const char * const aParam[], int aCount)
{
	return ProcessCommandInt(OPENGL_MULTISAMPLE, aParam, aCount, UpdateWindowAction, "multisample: %d\n");
}
Command commandmultisample(0x47d0f228 /* "multisample" */, CommandMultisample);

int CommandViewSize(const char * const aParam[], int aCount)
{
	return ProcessCommandFloat(VIEW_SIZE, aParam, aCount, NULL, "viewsize: %f\n");
}
Command commandviewsize(0x1ae79789 /* "viewsize" */, CommandViewSize);

int CommandInput(const char * const aParam[], int aCount)
{
	return ProcessCommandString(INPUT_CONFIG, aParam, aCount, InitInputAction, "input: %s\n");
}
Command commandinput(0xf9d86f7b /* "input" */, CommandInput);

int CommandLevel(const char * const aParam[], int aCount)
{
	return ProcessCommandString(LEVEL_CONFIG, aParam, aCount, InitLevelAction, "level: %s\n");
}
Command commandlevel(0x9b99e7dd /* "level" */, CommandLevel);

int CommandImport(const char * const aParam[], int aCount)
{
	if (aCount > 0)
	{
		// level configuration
		tinyxml2::XMLDocument document;
		if (document.LoadFile(aParam[0]) != tinyxml2::XML_SUCCESS)
			DebugPrint("error loading import file \"%s\": %s %s\n", aParam[0], document.GetErrorStr1(), document.GetErrorStr2());

		// process child element
		if (const tinyxml2::XMLElement *root = document.FirstChildElement())
			ConfigureWorldItem(root);
		return 1;
	}
	return 0;
}
Command commandimport(0x112a90d4 /* "import" */, CommandImport);

int CommandSplit(const char * const aParam[], int aCount)
{
	if (aCount > 0)
	{
		SplitLevel(LEVEL_CONFIG.c_str(), aParam[0]);
		return 1;
	}
	return 0;
}
Command commandsplit(0x87b82de3 /* "split" */, CommandSplit);

int CommandMerge(const char * const aParam[], int aCount)
{
	if (aCount > 0)
	{
		MergeLevel(LEVEL_CONFIG.c_str(), aParam[0]);
		return 1;
	}
	return 0;
}
Command commandmerge(0xb9764627 /* "merge" */, CommandMerge);

int CommandRecord(const char * const aParam[], int aCount)
{
	return ProcessCommandString(RECORD_CONFIG, aParam, aCount, InitRecordAction, "record: %s\n");
}
Command commandrecord(0x593058cc /* "record" */, CommandRecord);

int CommandPlayback(const char * const aParam[], int aCount)
{
	return ProcessCommandString(RECORD_CONFIG, aParam, aCount, InitPlaybackAction, "playback: %s\n");
}
Command commandplayback(0xcf8a43ec /* "playback" */, CommandPlayback);

int CommandSimRate(const char * const aParam[], int aCount)
{
	return ProcessCommandInt(SIMULATION_RATE, aParam, aCount, NULL, "simrate: %d\n");
}
Command commandsimrate(0xd6974b06 /* "simrate" */, CommandSimRate);

int CommandTimeScale(const char * const aParam[], int aCount)
{
	return ProcessCommandFloat(TIME_SCALE, aParam, aCount, NULL, "timescale: %f\n");
}
Command commandtimescale(0x9f2f269e /* "timescale" */, CommandTimeScale);

int CommandFixedStep(const char * const aParam[], int aCount)
{
	return ProcessCommandBool(FIXED_STEP, aParam, aCount, NULL, "fixedstep: %d\n");
}
Command commandfixedstep(0xe065cb63 /* "fixedstep" */, CommandFixedStep);

int CommandMotionBlur(const char * const aParam[], int aCount)
{
	return ProcessCommandInt(MOTIONBLUR_STEPS, aParam, aCount, ClampMotionBlurAction, "motionblur: %d\n");
}
Command commandmotionblur(0xf744f3b2 /* "motionblur" */, CommandMotionBlur);

int CommandMotionBlurTime(const char * const aParam[], int aCount)
{
	return ProcessCommandFloat(MOTIONBLUR_TIME, aParam, aCount, NULL, "motionblurtime: %f\n");
}
Command commandmotionblurtime(0xfb585f73 /* "motionblurtime" */, CommandMotionBlurTime);

int CommandSoundChannels(const char * const aParam[], int aCount)
{
	return ProcessCommandInt(SOUND_CHANNELS, aParam, aCount, NULL, "soundchannels: %d\n");
}
Command commandsoundchannels(0x61e734dc /* "soundchannels" */, CommandSoundChannels);

int CommandSoundVolumeEffect(const char * const aParam[], int aCount)
{
	return ProcessCommandFloat(SOUND_VOLUME_EFFECT, aParam, aCount, UpdateSoundVolume, "soundvolumemusic: %f\n");
}
Command commandsoundvolumeeffect(0xff4838bb /* "soundvolumeeffect" */, CommandSoundVolumeEffect);

int CommandSoundVolumeMusic(const char * const aParam[], int aCount)
{
	return ProcessCommandFloat(SOUND_VOLUME_MUSIC, aParam, aCount, UpdateMusicVolume, "soundvolumemusic: %f\n");
}
Command commandsoundvolumemusic(0x689fc51d /* "soundvolumemusic" */, CommandSoundVolumeMusic);

int CommandOutputConsole(const char * const aParam[], int aCount)
{
	return ProcessCommandBool(DEBUGPRINT_OUTPUTCONSOLE, aParam, aCount, NULL, "outputconsole: %d\n");
}
Command commandoutputconsole(0x94c716fd /* "outputconsole" */, CommandOutputConsole);

int CommandOutputDebug(const char * const aParam[], int aCount)
{
	return ProcessCommandBool(DEBUGPRINT_OUTPUTDEBUG, aParam, aCount, NULL, "outputdebug: %d\n");
}
Command commandoutputdebug(0x54822903 /* "outputdebug" */, CommandOutputDebug);

int CommandOutputStdErr(const char * const aParam[], int aCount)
{
	return ProcessCommandBool(DEBUGPRINT_OUTPUTSTDERR, aParam, aCount, NULL, "outputstderr: %d\n");
}
Command commandoutputstderr(0x8940763c /* "outputstderr" */, CommandOutputStdErr);

int CommandProfileScreen(const char * const aParam[], int aCount)
{
	return ProcessCommandBool(PROFILER_OUTPUTSCREEN, aParam, aCount, NULL, "profilescreen: %d\n");
}
Command commandprofilescreen(0xfbcc8f02 /* "profilescreen" */, CommandProfileScreen);

int CommandProfilePrint(const char * const aParam[], int aCount)
{
	return ProcessCommandBool(PROFILER_OUTPUTPRINT, aParam, aCount, NULL, "profileprint: %d\n");
}
Command commandprofileprint(0x85e872f9 /* "profileprint" */, CommandProfilePrint);

int CommandFrameRateScreen(const char * const aParam[], int aCount)
{
	return ProcessCommandBool(FRAMERATE_OUTPUTSCREEN, aParam, aCount, NULL, "frameratescreen: %d\n");
}
Command commandframeratescreen(0x24ce5450 /* "frameratescreen" */, CommandFrameRateScreen);

int CommandFrameRatePrint(const char * const aParam[], int aCount)
{
	return ProcessCommandBool(FRAMERATE_OUTPUTPRINT, aParam, aCount, NULL, "framerateprint: %d\n");
}
Command commandframerateprint(0x55cfbc33 /* "framerateprint" */, CommandFrameRatePrint);

int CommandDebugDraw(const char * const aParam[], int aCount)
{
	return ProcessCommandBool(DEBUG_DRAW, aParam, aCount, NULL, "debugdraw: %d\n");
}
Command commanddebugdraw(0xe41f87fa /* "debugdraw" */, CommandDebugDraw);

int CommandDatabase(const char * const aParam[], int aCount)
{
	if (aCount >= 1)
	{
		// get the database identifier
		unsigned int id;
		if (!TIXML_SSCANF(aParam[0], "0x%x", &id))
			id = Hash(aParam[0]);

		// get the dtabase
		Database::Untyped *db = Database::GetDatabases().Get(id);
		if (db)
		{
			// list database properties
			console->Print("stride=%d chunk=%d limit=%d count=%d\n",
				db->GetStride(), db->GetChunk(), db->GetLimit(), db->GetCount());
		}
		else
		{
			// not found
			console->Print("database \"%s\" (0x%08x) not found\n", aParam[0], id);
		}
		return 1;
	}
	else
	{
		// list all database identifiers
		console->Print("databases:\n");
		for (Database::Untyped::Iterator itor(&Database::GetDatabases()); itor.IsValid(); ++itor)
		{
			console->Print("0x%08x\n", itor.GetKey());
		}
		return 0;
	}
}
Command commanddatabase(0xa165ddb8 /* "database" */, CommandDatabase);

int CommandFind(const char * const aParam[], int aCount)
{
	if (aCount >= 1)
	{
		// get the database identifier
		unsigned int id;
		if (!TIXML_SSCANF(aParam[0], "0x%x", &id))
			id = Hash(aParam[0]);

		// get the database
		Database::Untyped *db = Database::GetDatabases().Get(id);
		if (db)
		{
			// if supplying a key...
			if (aCount >= 2)
			{
				// if the key is an identifier...
				unsigned int key = 0;
				if (TIXML_SSCANF(aParam[1], "0x%x", &key))
				{
					// look up the identifier
					if (const void *value = db->Find(key))
					{
						const std::string &name = Database::name.Get(key);
						console->Print("name=\"%s\" key=0x%08x data=0x%p\n", name.c_str(), key, value);
					}
					else
					{
						console->Print("no record found\n");
					}
				}
				else
				{
					// list all keys matching the string name
					console->Print("records matching \"%s\":\n", aParam[1]);
					for (Database::Untyped::Iterator itor(db); itor.IsValid(); ++itor)
					{
						const std::string &name = Database::name.Get(itor.GetKey());
						if (_stricmp(name.c_str(), aParam[1]) == 0)
						{
							console->Print("%d: name=\"%s\" key=0x%08x data=0x%p\n", itor.GetSlot(), name.c_str(), itor.GetKey(), itor.GetValue());
						}
					}
				}
				return 2;
			}
			else
			{
				// list the contents of the database
				for (Database::Untyped::Iterator itor(db); itor.IsValid(); ++itor)
				{
					const std::string &name = Database::name.Get(itor.GetKey());
					console->Print("%d: name=\"%s\" key=0x%08x data=0x%p\n", itor.GetSlot(), name.c_str(), itor.GetKey(), itor.GetValue());
				}
			}
		}
		else
		{
			// not found
			console->Print("database \"%s\" (0x%08x) not found\n", aParam[0], id);
		}
		return 1;
	}
	else
	{
		return 0;
	}
}
Command commandfind(0xbdf0855a /* "find" */, CommandFind);

int CommandComponents(const char * const aParam[], int aCount)
{
	if (aCount >= 1)
	{
		// get the identifier
		unsigned int key;
		if (!TIXML_SSCANF(aParam[0], "0x%x", &key))
			key = Hash(aParam[0]);

		// for each database...
		for (Database::Typed<Database::Untyped *>::Iterator itor(&Database::GetDatabases()); itor.IsValid(); ++itor)
		{
			// if the database contains a record for the identifier...
			if (const void *value = itor.GetValue()->Find(key))
			{
				console->Print("database 0x%08x data=0x%p\n", itor.GetKey(), value);
			}
		}
		return 1;
	}
	else
	{
		return 0;
	}
}
Command commandcomponents(0x1bf51169 /* "components" */, CommandComponents);

int CommandSound(const char * const aParam[], int aCount)
{
	if (aCount >= 2)
	{
		PlaySoundCue(Hash(aParam[0]), Hash(aParam[1]));
		return 2;
	}
	else if (aCount == 1)
	{
		PlaySoundCue(Hash(aParam[0]));
		return 1;
	}
	else
	{
		return 0;
	}
	
}
Command commandsound(0x0e0d9594 /* "sound" */, CommandSound);

// commands
int ProcessCommand( unsigned int aCommand, char *aParam[], int aCount )
{
	const Command::Entry &entry = Command::Get(aCommand);
	if (entry)
	{
		return entry(aParam, aCount);
	}
#ifdef USE_VARIABLE
	// check variable system
	if (VarItem *item = Database::varitem.Get(aCommand))
	{
		if (item->mType == VarItem::COMMAND)
		{
			// trigger the command
			item->Notify();
			return 0;
		}
		else
		{
			if (aCount >= 1)
			{
				// set value
				item->SetString(aParam[0]);
				return 1;
			}
			else
			{
				// get value
				console->Print("%s\n", item->GetString().c_str());
				return 0;
			}
		}
	}
	else
	{
		return 0;
	}
#else
	return 0;
#endif
}
