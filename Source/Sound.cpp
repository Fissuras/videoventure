#include "StdAfx.h"
#include "Sound.h"
#include "Entity.h"

#ifdef USE_POOL_ALLOCATOR
#include <boost/pool/pool.hpp>

// sound pool
static boost::pool<boost::default_user_allocator_malloc_free> pool(sizeof(Sound));
void *Sound::operator new(size_t aSize)
{
	return pool.malloc();
}
void Sound::operator delete(void *aPtr)
{
	pool.free(aPtr);
}
#endif

namespace Database
{
	Typed<SoundTemplate> soundtemplate(0x1b5ef1be /* "soundtemplate" */);
	Typed<Typed<unsigned int> > soundcue(0xf23cbd5f /* "soundcue" */);
	Typed<Typed<Sound *> > sound(0x0e0d9594 /* "sound" */);

	namespace Loader
	{
		class SoundLoader
		{
		public:
			SoundLoader()
			{
				AddConfigure(0x0e0d9594 /* "sound" */, Entry(this, &SoundLoader::Configure));
			}

			void Configure(unsigned int aId, const TiXmlElement *element)
			{
				// open sound template
				SoundTemplate &sound = Database::soundtemplate.Open(aId);

				element->QueryFloatAttribute("volume", &sound.mVolume);
				element->QueryIntAttribute("repeat", &sound.mRepeat);

				// add a default cue (HACK)
				Typed<unsigned int> &soundcue = Database::soundcue.Open(aId);
				soundcue.Put(0, aId);
				Database::soundcue.Close(aId);

				// process sound configuration
				for (const TiXmlElement *child = element->FirstChildElement(); child; child = child->NextSiblingElement())
				{
					switch (Hash(child->Value()))
					{
					case 0xaaea5743 /* "file" */:
						{
							const char *name = child->Attribute("name");
							if (name)
							{
								// load wave file data
								SDL_AudioSpec wave;
								Uint8 *data;
								Uint32 dlen;
								if ( !SDL_LoadWAV(name, &wave, &data, &dlen) )
								{
									DebugPrint("error loading sound file \"%s\": %s\n", name, SDL_GetError());
									continue;
								}

								// build audio conversion
								SDL_AudioCVT cvt;
								SDL_BuildAudioCVT(&cvt, wave.format, wave.channels, wave.freq,
														AUDIO_S16,   2,             AUDIO_FREQUENCY);

#if 1
								// replace sound data
								sound.mData = static_cast<unsigned char *>(realloc(sound.mData, dlen * cvt.len_mult));
								sound.mLength = 0;
#else
								// append sound data
								sound.mData = static_cast<unsigned char *>(realloc(sound.mData, sound.mLength + dlen * cvt.len_mult));
#endif
								memcpy(sound.mData + sound.mLength, data, dlen);
								cvt.buf = sound.mData + sound.mLength;
								cvt.len = dlen;

								// convert to final format
								SDL_ConvertAudio(&cvt);
								sound.mLength += cvt.len_cvt;
								sound.mData = static_cast<unsigned char *>(realloc(sound.mData, sound.mLength));

								// release wave file data
								SDL_FreeWAV(data);
							}
						}
					}
				}

				// close sound template
				Database::soundtemplate.Close(aId);
			}
		}
		soundloader;

		class SoundCueLoader
		{
		public:
			SoundCueLoader()
			{
				AddConfigure(0xf23cbd5f /* "soundcue" */, Entry(this, &SoundCueLoader::Configure));
			}

			void Configure(unsigned int aId, const TiXmlElement *element)
			{
				// open soundcue
				Typed<unsigned int> &soundcue = Database::soundcue.Open(aId);

				// if the object has an embedded sound...
				if (Database::soundtemplate.Find(aId))
				{
					// add a default cue (HACK)
					soundcue.Put(0, aId);
				}

				// process sound cue configuration
				for (const TiXmlElement *child = element->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
				{
					switch (Hash(child->Value()))
					{
					case 0xe5561300 /* "cue" */:
						{
							// assign cue
							unsigned int subid = Hash(child->Attribute("name"));
							unsigned int &cue = soundcue.Open(subid);
							cue = Hash(child->Attribute("sound"));
							soundcue.Close(subid);
						}
						break;
					}
				}

				// close soundcue
				Database::soundcue.Close(aId);
			}
		}
		soundcueloader;
	}

	namespace Initializer
	{
		class SoundInitializer
		{
		public:
			SoundInitializer()
			{
				AddActivate(0x1b5ef1be /* "soundtemplate" */, Entry(this, &SoundInitializer::Activate));
				AddDeactivate(0x1b5ef1be /* "soundtemplate" */, Entry(this, &SoundInitializer::Deactivate));
			}

			void Activate(unsigned int aId)
			{
				PlaySound(aId, 0);
			}

			void Deactivate(unsigned int aId)
			{
				if (Database::sound.Find(aId))
				{
					SDL_LockAudio();
					const Typed<Sound *> &sounds = Database::sound.Get(aId);
					for (Typed<Sound *>::Iterator itor(&sounds); itor.IsValid(); ++itor)
						delete itor.GetValue();
					Database::sound.Delete(aId);
					SDL_UnlockAudio();
				}
			}
		}
		soundinitializer;
	}
}

SoundTemplate::SoundTemplate(void)
: mData(NULL)
, mLength(0)
, mVolume(1.0f)
, mRepeat(0)
{
}

SoundTemplate::SoundTemplate(const SoundTemplate &aTemplate)
: mData(static_cast<unsigned char *>(malloc(aTemplate.mLength)))
, mLength(aTemplate.mLength)
, mVolume(aTemplate.mVolume)
, mRepeat(aTemplate.mRepeat)
{
	memcpy(mData, aTemplate.mData, mLength);
}

SoundTemplate::~SoundTemplate(void)
{
	free(mData);
}


static Sound *sHead;
static Sound *sTail;
static Sound *sNext;

Sound::Sound(void)
: Updatable(0)
, mNext(NULL)
, mPrev(NULL)
, mData(NULL)
, mLength(0)
, mOffset(0)
, mVolume(0)
, mRepeat(0)
, mPosition(0, 0)
, mPlaying(false)
{
}

Sound::Sound(const SoundTemplate &aTemplate, unsigned int aId)
: Updatable(aId)
, mNext(NULL)
, mPrev(NULL)
, mData(aTemplate.mData)
, mLength(aTemplate.mLength)
, mOffset(0)
, mVolume(aTemplate.mVolume)
, mRepeat(aTemplate.mRepeat)
, mPosition(0, 0)
, mPlaying(false)
{
}

Sound::~Sound(void)
{
	Stop();
}


void Sound::Play(unsigned int aOffset)
{
	if (!mPlaying)
	{
		SDL_LockAudio();
		mPlaying = true;
		mPrev = sTail;
		if (sTail)
			sTail->mNext = this;
		sTail = this;
		if (!sHead)
			sHead = this;
		SDL_UnlockAudio();
	}

	mOffset = aOffset;

	// also activate
	Activate();
}

void Sound::Stop(void)
{
	if (mPlaying)
	{
		SDL_LockAudio();
		mPlaying = false;
		if (sHead == this)
			sHead = mNext;
		if (sTail == this)
			sTail = mPrev;
		if (sNext == this)
			sNext = mNext;
		if (mNext)
			mNext->mPrev = mPrev;
		if (mPrev)
			mPrev->mNext = mNext;
		mNext = NULL;
		mPrev = NULL;
		SDL_UnlockAudio();
	}

	// also deactivate
	Deactivate();
}

// hack!
extern Vector2 listenerpos;
void Sound::Update(float aStep)
{
	if (!mRepeat && mOffset >= mLength)
	{
		Stop();
		return;
	}

	if (Entity *entity = Database::entity.Get(id))
		mPosition = entity->GetPosition();
	else
		mPosition = listenerpos;
}


// AUDIO MIXER

static const int MAX_CHANNELS = 4;

static const float timestep = 1.0f / AUDIO_FREQUENCY;
static float average0 = 0.0f, average1 = 0.0f;
static const float averagefilter = 1.0f * timestep;
static const float minlevel = 32768.0f*32768.0f;
static float level = minlevel;
static const float levelfilter = 1.0f * timestep;
static const float postscale = 65534.0f / float(M_PI);

void Sound::Mix(void *userdata, Uint8 *stream, int len)
{
	int samples = len / sizeof(short);

	// if no sounds playing...
	if (sHead == NULL)
	{
		// update filters
		average0 -= average0 * 0.5f * samples * averagefilter;
		average1 -= average1 * 0.5f * samples * averagefilter;
		level -= level * 0.5f * samples * levelfilter;
		if (level < minlevel)
			level = minlevel;
		return;
	}

	// custom mixer
	static float mix[2048];
	memset(mix, 0, samples * sizeof(float));

	// listener position
	Vector2 &listenerpos = *static_cast<Vector2 *>(userdata);

	// sound channels
	unsigned char *channel_data[MAX_CHANNELS+1] = { 0 };
	unsigned int channel_offset[MAX_CHANNELS+1] = { 0 };
	unsigned int channel_length[MAX_CHANNELS+1] = { 0 };
	unsigned int channel_repeat[MAX_CHANNELS+1] = { 0 };
	float channel_volume[MAX_CHANNELS+1] = { 0 };
	float channel_weight[MAX_CHANNELS+1] = { 0 };
	int channel_count = 0;

	// for each active sound...
	for (Sound *sound = sHead; sound != NULL; sound = sound->mNext)
	{
		// apply sound fall-off
		float volume = sound->mVolume / (1.0f + listenerpos.DistSq(sound->mPosition) / 16384.0f);
		if (volume < 1.0f/256.0f)
			continue;

		// weight sound based on volume
		float weight = volume;

		// if not repeating...
		if (!sound->mRepeat)
		{
			// diminish weight over time
			weight *= (1.0f - sound->mOffset / sound->mLength);
		}

		// update channel data
		int j;
		for (j = channel_count - 1; j >= 0; j--)
		{
			if (channel_weight[j] >= weight) break;
			channel_data[j + 1] = channel_data[j];
			channel_offset[j + 1] = channel_offset[j];
			channel_length[j + 1] = channel_length[j];
			channel_repeat[j + 1] = channel_repeat[j];
			channel_volume[j + 1] = channel_volume[j];
			channel_weight[j + 1] = channel_weight[j];
		}
		if (j < MAX_CHANNELS - 1)
		{
			channel_data[j + 1] = sound->mData;
			channel_offset[j + 1] = sound->mOffset;
			channel_length[j + 1] = sound->mLength;
			channel_repeat[j + 1] = sound->mRepeat;
			channel_volume[j + 1] = volume;
			channel_weight[j + 1] = weight;
			if (channel_count < MAX_CHANNELS)
				++channel_count;
		}
	}

	// for each active channel...
	for (int channel = 0; channel < channel_count; ++channel)
	{
		// get starting offset
		unsigned char *data = channel_data[channel];
		unsigned int length = channel_length[channel];
		unsigned int offset = channel_offset[channel];
		float volume = channel_volume[channel];

		// while output to generate...
		int output = 0;
		while (output < samples)
		{
			// calculate amount to output
			int amount = std::min(size_t(length-offset)/sizeof(short), size_t(samples-output));

			// add volume-scaled sample
			for (int i = 0; i < amount; ++i)
				mix[output+i] += ((short *)(data+offset))[i] * volume;

			// advance offset
			offset += amount * sizeof(short);

			// if reaching the end...
			if (offset >= length)
			{
				// if repeating...
				if (channel_repeat[channel])
				{
					// loop around
					offset -= length;
				}
				else
				{
					// stop
					break;
				}
			}

			// advance output position
			output += amount;
		}
	}

	// for each active sound...
	for (Sound *sound = sHead; sound != NULL; sound = sound->mNext)
	{
		// update sound position
		sound->mOffset += len;

		// if moving past the end...
		if (sound->mOffset >= sound->mLength)
		{
			// if repeating
			if (sound->mRepeat)
			{
				// loop the sound
				sound->mOffset %= sound->mLength;
			}
			else
			{
				// clamp to the end
				sound->mOffset = sound->mLength;
			}
		}
	}

	// subract filtered average to remove DC term
	// apply filtered scaling to compress dynamic range
	// apply nonlinear curve to eliminate clipping
	float *src = mix;
	short *dst = (short *)stream;
	for (int i = 0; i < samples/2; ++i)
	{
		float mix0 = *src++;
		float mix1 = *src++;
		average0 += (mix0 -= average0) * averagefilter;
		average1 += (mix1 -= average1) * averagefilter;
		level += (mix0 * mix0 + mix1 * mix1 - level) * levelfilter;
		if (level < minlevel)
			level = minlevel;
		float prescale = InvSqrt(level);
		*dst++ = short(atanf(mix0 * prescale) * postscale);
		*dst++ = short(atanf(mix1 * prescale) * postscale);
	}
}

void PlaySound(unsigned int aId, unsigned int aCueId)
{
	const Database::Typed<unsigned int> &soundcues = Database::soundcue.Get(aId);
	unsigned int aSoundId = soundcues.Get(aCueId);
	const SoundTemplate &soundtemplate = Database::soundtemplate.Get(aSoundId);
	if (soundtemplate.mData)
	{
		/* Put the sound data in the slot (it starts playing immediately) */
		Database::Typed<Sound *> &sounds = Database::sound.Open(aId);
		if (Sound *s = sounds.Get(aCueId))
		{
			// retrigger
			s->Play(0);
		}
		else
		{
			// start new
			s = new Sound(soundtemplate, aId);
			sounds.Put(aCueId, s);
			s->Play(0);
		}
	}
}
