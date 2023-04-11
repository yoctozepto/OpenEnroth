#include <algorithm>
#include <map>
#include <string>
#include <filesystem>
#include <utility>
#include <thread>

#include "Media/Audio/AudioPlayer.h"

#include "Library/Compression/Compression.h"

#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Level/Decoration.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Party.h"

#include "Media/Audio/OpenALSoundProvider.h"

int sLastTrackLengthMS;
AudioPlayer *pAudioPlayer;
SoundList *pSoundList;

std::array<float, 10> pSoundVolumeLevels = {
    {0.0000000f, 0.1099999f, 0.2199999f, 0.3300000f, 0.4399999f, 0.5500000f,
     0.6600000f, 0.7699999f, 0.8799999f, 0.9700000f}};

enum SOUND_TYPE {
    SOUND_TYPE_LEVEL = 0,
    SOUND_TYPE_SYSTEM = 1,
    SOUND_TYPE_SWAP = 2,
    SOUND_TYPE_UNKNOWN = 3,
    SOUND_TYPE_LOCK = 4,
};

enum SOUND_FLAG {
    SOUND_FLAG_LOCKED = 0x1,
    SOUND_FLAG_3D = 0x2,
};

// Max value used for volume control
// TODO(Nik-RE-dev): originally it was 2.0f, but OpenAL support gains from [0.0f, 1.0f] only
static const float maxVolumeGain = 1.0f;

// TODO(Nik-RE-dev): investigate importance of applying scaling to position coordinates
static const float positionScaling = 50.0f;

class SoundInfo {
 public:
    bool Is3D() { return ((uFlags & SOUND_FLAG_3D) == SOUND_FLAG_3D); }

 public:
    std::string sName;
    SOUND_TYPE eType;
    uint32_t uSoundID;
    uint32_t uFlags;
    std::shared_ptr<Blob> buffer;
    PAudioDataSource dataSource;
};

std::map<uint32_t, SoundInfo> mapSounds;

#pragma pack(push, 1)
struct SoundDesc_mm6 {
    uint8_t pSoundName[32];
    uint32_t uSoundID;
    uint32_t eType;
    uint32_t uFlags;
    uint32_t pSoundDataID[17];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct SoundDesc : public SoundDesc_mm6 {
    uint32_t p3DSoundID;
    uint32_t bDecompressed;
};
#pragma pack(pop)

void SoundList::Initialize() {}

void SoundList::FromFile(const Blob &data_mm6, const Blob &data_mm7, const Blob &data_mm8) {
    static_assert(sizeof(SoundDesc_mm6) == 112, "Wrong type size");
    static_assert(sizeof(SoundDesc) == 120, "Wrong type size");

    size_t num_mm6_sounds = data_mm6 ? *(uint32_t *)data_mm6.data() : 0;
    size_t num_mm7_sounds = data_mm7 ? *(uint32_t *)data_mm7.data() : 0;
    size_t num_mm8_sounds = data_mm8 ? *(uint32_t *)data_mm8.data() : 0;

    unsigned int sNumSounds = num_mm6_sounds + num_mm7_sounds + num_mm8_sounds;
    assert(sNumSounds);
    assert(!num_mm8_sounds);

    SoundDesc *sounds = (SoundDesc *)((char *)data_mm7.data() + 4);
    for (size_t i = 0; i < num_mm7_sounds; i++) {
        SoundInfo si;
        si.sName = (char *)sounds[i].pSoundName;
        si.uSoundID = sounds[i].uSoundID;
        si.eType = (SOUND_TYPE)sounds[i].eType;
        si.uFlags = sounds[i].uFlags;
        mapSounds[si.uSoundID] = si;
    }

    SoundDesc_mm6 *sounds_mm6 = (SoundDesc_mm6 *)((char *)data_mm6.data() + 4);
    for (size_t i = 0; i < num_mm6_sounds; i++) {
        SoundInfo si;
        si.sName = (char *)sounds_mm6[i].pSoundName;
        si.uSoundID = sounds_mm6[i].uSoundID;
        si.eType = (SOUND_TYPE)sounds_mm6[i].eType;
        si.uFlags = sounds_mm6[i].uFlags;
        mapSounds[si.uSoundID] = si;
    }
}

extern OpenALSoundProvider *provider;

void AudioPlayer::MusicPlayTrack(MusicID eTrack) {
    if (currentMusicTrack == eTrack) {
        return;
    }

    if (!engine->config->debug.NoSound.value() && bPlayerReady) {
        if (pCurrentMusicTrack) {
            pCurrentMusicTrack->Stop();
        }
        currentMusicTrack = MUSIC_Invalid;

        std::string file_path = fmt::format("{}.mp3", eTrack);
        file_path = MakeDataPath("music", file_path);
        if (!std::filesystem::exists(file_path)) {
            logger->warning("AudioPlayer: {} not found", file_path);
            return;
        }

        pCurrentMusicTrack = CreateAudioTrack(file_path);
        if (pCurrentMusicTrack) {
            currentMusicTrack = eTrack;

            pCurrentMusicTrack->SetVolume(uMusicVolume);
            pCurrentMusicTrack->Play();
            if (uMusicVolume == 0.0) {
                pCurrentMusicTrack->Pause();
            }
        }
    }
}

void AudioPlayer::MusicStart() {}

void AudioPlayer::MusicStop() {
    if (!pCurrentMusicTrack) {
        return;
    }

    pCurrentMusicTrack->Stop();
    pCurrentMusicTrack = nullptr;
    currentMusicTrack = MUSIC_Invalid;
}

void AudioPlayer::MusicPause() {
    if (!pCurrentMusicTrack) {
        return;
    }

    pCurrentMusicTrack->Pause();
}

void AudioPlayer::MusicResume() {
    if (!pCurrentMusicTrack) {
        return;
    }

    if (!pCurrentMusicTrack->Resume()) {
        int playedMusicTrack = currentMusicTrack;
        if (currentMusicTrack != MUSIC_Invalid) {
            MusicStop();
            MusicPlayTrack((MusicID)playedMusicTrack);
        }
    }
}

void AudioPlayer::SetMusicVolume(int level) {
    if (!pCurrentMusicTrack) {
        return;
    }

    level = std::max(0, level);
    level = std::min(9, level);
    uMusicVolume = pSoundVolumeLevels[level] * maxVolumeGain;
    pCurrentMusicTrack->SetVolume(uMusicVolume);
    if (level == 0) {
        MusicPause();
    } else {
        MusicResume();
    }
}

void AudioPlayer::SetMasterVolume(int level) {
    level = std::max(0, level);
    level = std::min(9, level);
    uMasterVolume = (maxVolumeGain * pSoundVolumeLevels[level]);

    _regularSoundPool.setVolume(uMasterVolume);
    _loopingSoundPool.setVolume(uMasterVolume);
    if (_currentWalkingSample) {
        _currentWalkingSample->SetVolume(uMasterVolume);
    }
}

void AudioPlayer::SetVoiceVolume(int level) {
    level = std::max(0, level);
    level = std::min(9, level);
    uVoiceVolume = (maxVolumeGain * pSoundVolumeLevels[level]);

    _voiceSoundPool.setVolume(uVoiceVolume);
}

void AudioPlayer::stopSounds() {
    if (!bPlayerReady) {
        return;
    }

    _voiceSoundPool.stop();
    _regularSoundPool.stop();
    _loopingSoundPool.stop();
    if (_currentWalkingSample) {
        _currentWalkingSample->Stop();
        _currentWalkingSample = nullptr;
    }
}

void AudioPlayer::stopVoiceSounds() {
    if (!bPlayerReady) {
        return;
    }

    _voiceSoundPool.stop();
}

void AudioPlayer::stopWalkingSounds() {
    if (!bPlayerReady) {
        return;
    }

    if (_currentWalkingSample) {
        _currentWalkingSample->Stop();
        _currentWalkingSample = nullptr;
    }
}

void AudioPlayer::resumeSounds() {
    _voiceSoundPool.resume();
    _regularSoundPool.resume();
    _loopingSoundPool.resume();
    if (_currentWalkingSample) {
        _currentWalkingSample->Resume();
    }
}

void AudioPlayer::playSound(SoundID eSoundID, int pid, unsigned int uNumRepeats, int source_x, int source_y, int sound_data_id) {
    if (!bPlayerReady)
        return;

    //logger->Info("AudioPlayer: trying to load sound id {}", eSoundID);

    if (engine->config->settings.SoundLevel.value() < 1 || (eSoundID == SOUND_Invalid)) {
        return;
    }

    if (mapSounds.find(eSoundID) == mapSounds.end()) {
        logger->warning("AudioPlayer: sound id {} not found", eSoundID);
        return;
    }

    SoundInfo &si = mapSounds[eSoundID];
    //logger->Info("AudioPlayer: sound id {} found as '{}'", eSoundID, si.sName);

    if (!si.dataSource) {
        Blob buffer;

        if (si.sName == "") {  // enable this for bonus sound effects
            //logger->Info("AudioPlayer: trying to load bonus sound {}", eSoundID);
            //buffer = LoadSound(int(eSoundID));
        } else {
            buffer = LoadSound(si.sName);
        }

        if (!buffer) {
            logger->warning("AudioPlayer: failed to load sound {} ({})", eSoundID, si.sName);
            return;
        }

        si.dataSource = CreateAudioBufferDataSource(std::move(buffer));
        if (!si.dataSource) {
            logger->warning("AudioPlayer: failed to create sound data source {} ({})", eSoundID, si.sName);
            return;
        }
    }

    PAudioSample sample = CreateAudioSample(si.dataSource);

    bool result = true;
    sample->SetVolume(uMasterVolume);

    if (pid == 0) {  // generic sound like from UI
        result = _regularSoundPool.playNew(sample);
    } else if (pid == PID_INVALID) { // exclusive sounds - can override
        _regularSoundPool.stopSoundId(eSoundID);
        result = _regularSoundPool.playUniqueSoundId(sample, eSoundID);
    } else if (pid == -1) { // all instances must be changed to PID_INVALID
        assert(false && "AudioPlayer::playSound - pid == -1 is encountered.");
        _regularSoundPool.stopSoundId(eSoundID);
        result = _regularSoundPool.playUniqueSoundId(sample, eSoundID);
    } else if (pid == SOUND_PID_NON_RESETABLE) {  // exclusive sounds - no override (close chest)
        result = _regularSoundPool.playUniqueSoundId(sample, eSoundID);
    } else if (pid == SOUND_PID_WALKING) {
        if (_currentWalkingSample) {
            _currentWalkingSample->Stop();
        }
        _currentWalkingSample = sample;
        _currentWalkingSample->Play();
    } else if (pid == SOUND_PID_MUSIC_VOLUME) {
        sample->SetVolume(uMusicVolume);
        _regularSoundPool.stopSoundId(eSoundID);
        result = _regularSoundPool.playUniqueSoundId(sample, eSoundID);
    } else if (pid == SOUND_PID_VOICE_VOLUME) {
        sample->SetVolume(uVoiceVolume);
        _regularSoundPool.stopSoundId(eSoundID);
        result = _regularSoundPool.playUniqueSoundId(sample, eSoundID);
    } else {
        ObjectType object_type = PID_TYPE(pid);
        unsigned int object_id = PID_ID(pid);
        switch (object_type) {
            case OBJECT_Door: {
                assert(uCurrentlyLoadedLevelType == LEVEL_Indoor);
                assert((int)object_id < pIndoor->pDoors.size());

                sample->SetPosition(pIndoor->pDoors[object_id].pXOffsets[0] / positionScaling,
                                    pIndoor->pDoors[object_id].pYOffsets[0] / positionScaling,
                                    pIndoor->pDoors[object_id].pZOffsets[0] / positionScaling, 500.f);

                // Door sounds are "looped" for the duration of moving animation by calling sound play each frame
                // so need to actually start new playing after previous sound finished playing.
                result = _regularSoundPool.playUniquePid(sample, pid, true);

                break;
            }

            case OBJECT_Player: {
                sample->SetVolume(uVoiceVolume);
                result = _voiceSoundPool.playUniquePid(sample, pid);

                break;
            }

            case OBJECT_Actor: {
                assert(object_id < pActors.size());

                sample->SetPosition(pActors[object_id].vPosition.x / positionScaling,
                                    pActors[object_id].vPosition.y / positionScaling,
                                    pActors[object_id].vPosition.z / positionScaling, 500.f);

                result = _regularSoundPool.playUniquePid(sample, pid, true);

                break;
            }

            case OBJECT_Decoration: {
                assert(object_id < pLevelDecorations.size());

                // TODO(Nik-RE-dev): why distance for decorations is 4 times more that for other sounds?
                sample->SetPosition((float)pLevelDecorations[object_id].vPosition.x / positionScaling,
                                    (float)pLevelDecorations[object_id].vPosition.y / positionScaling,
                                    (float)pLevelDecorations[object_id].vPosition.z / positionScaling, 2000.f);

                result = _loopingSoundPool.playNew(sample, true);

                break;
            }

            case OBJECT_Item: {
                assert(object_id < pSpriteObjects.size());

                sample->SetPosition(pSpriteObjects[object_id].vPosition.x / positionScaling,
                                    pSpriteObjects[object_id].vPosition.y / positionScaling,
                                    pSpriteObjects[object_id].vPosition.z / positionScaling, 500.f);

                result = _regularSoundPool.playNew(sample, true);
                break;
            }

            case OBJECT_Face: {
                result = _regularSoundPool.playNew(sample);

                break;
            }

            default: {
                // TODO(pskelton): temp fix to reduce instances of sounds not playing
                result = _regularSoundPool.playNew(sample);
                logger->verbose("Unexpected object type from PID in playSound");
                break;
            }
        }
    }

    if (!result) {
        if (si.sName.empty()) {
            logger->warning("AudioPlayer: failed to play audio {} with name '{}'", eSoundID, si.sName);
        } else {
            logger->warning("AudioPlayer: failed to play audio {}", eSoundID);
        }
    } else {
        if (si.sName.empty()) {
            logger->verbose("AudioPlayer: playing sound {}", eSoundID);
        } else {
            logger->verbose("AudioPlayer: playing sound {} with name '{}'", eSoundID, si.sName);
        }
    }
}

void AudioPlayer::UpdateSounds() {
    float pitch = pi * (float)pParty->_viewPitch / 1024.f;
    float yaw = pi * (float)pParty->_viewYaw / 1024.f;

    provider->SetOrientation(yaw, pitch);
    provider->SetListenerPosition(pParty->vPosition.x / positionScaling,
                                  pParty->vPosition.y / positionScaling,
                                  pParty->vPosition.z / positionScaling);

    _voiceSoundPool.update();
    _regularSoundPool.update();
    _loopingSoundPool.update();
    if (_currentWalkingSample && _currentWalkingSample->IsStopped()) {
        _currentWalkingSample = nullptr;
    }
}

void AudioPlayer::pauseAllSounds() {
    _voiceSoundPool.pause();
    _regularSoundPool.pause();
    _loopingSoundPool.pause();
    if (_currentWalkingSample) {
        _currentWalkingSample->Pause();
    }
}

void AudioPlayer::pauseLooping() {
    _loopingSoundPool.pause();
}

void AudioPlayer::soundDrain() {
    while (_voiceSoundPool.hasPlaying()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        _voiceSoundPool.update();
    }
    while (_regularSoundPool.hasPlaying()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        _regularSoundPool.update();
    }
}

bool AudioSamplePool::playNew(PAudioSample sample, bool positional) {
    update();
    /*if (!sample->Open()) {
        return false;
    }*/
    sample->Play(_looping, positional);
    _samplePool.push_back(AudioSamplePoolEntry(sample, SOUND_Invalid, PID_INVALID));
    return true;
}

bool AudioSamplePool::playUniqueSoundId(PAudioSample sample, SoundID id, bool positional) {
    update();
    for (AudioSamplePoolEntry &entry : _samplePool) {
        if (entry.id == id) {
            return true;
        }
    }
    /*if (!sample->Open()) {
        return false;
    }*/
    sample->Play(_looping, positional);
    _samplePool.push_back(AudioSamplePoolEntry(sample, id, PID_INVALID));
    return true;
}

bool AudioSamplePool::playUniquePid(PAudioSample sample, int pid, bool positional) {
    update();
    for (AudioSamplePoolEntry &entry : _samplePool) {
        if (entry.pid == pid) {
            return true;
        }
    }
    /*if (!sample->Open()) {
        return false;
    }*/
    sample->Play(_looping, positional);
    _samplePool.push_back(AudioSamplePoolEntry(sample, SOUND_Invalid, pid));
    return true;
}

void AudioSamplePool::pause() {
    update();
    for (AudioSamplePoolEntry &entry : _samplePool) {
        entry.samplePtr->Pause();
    }
}

void AudioSamplePool::resume() {
    update();
    for (AudioSamplePoolEntry &entry : _samplePool) {
        entry.samplePtr->Resume();
    }
}

void AudioSamplePool::stop() {
    for (AudioSamplePoolEntry &entry : _samplePool) {
        entry.samplePtr->Stop();
    }
    _samplePool.clear();
}

void AudioSamplePool::stopSoundId(SoundID soundId) {
    assert(soundId != SOUND_Invalid);

    auto it = _samplePool.begin();
    while (it != _samplePool.end()) {
        if ((*it).id == soundId) {
            (*it).samplePtr->Stop();
            it = _samplePool.erase(it);
        } else {
            it++;
        }
    }
}

void AudioSamplePool::stopPid(int pid) {
    assert(pid != PID_INVALID);

    auto it = _samplePool.begin();
    while (it != _samplePool.end()) {
        if ((*it).pid == pid) {
            (*it).samplePtr->Stop();
            it = _samplePool.erase(it);
        } else {
            it++;
        }
    }
}

void AudioSamplePool::update() {
    auto it = _samplePool.begin();
    std::erase_if(_samplePool, [](const AudioSamplePoolEntry& entry) { return entry.samplePtr->IsStopped(); });
}

void AudioSamplePool::setVolume(float value) {
    for (AudioSamplePoolEntry &entry : _samplePool) {
        entry.samplePtr->SetVolume(value);
    }
}

bool AudioSamplePool::hasPlaying() {
    for (AudioSamplePoolEntry &entry : _samplePool) {
        if (!entry.samplePtr->IsStopped()) {
            return true;
        }
    }
    return false;
}

#pragma pack(push, 1)
struct SoundHeader_mm7 {
    char pSoundName[40];
    uint32_t uFileOffset;
    uint32_t uCompressedSize;
    uint32_t uDecompressedSize;
};
#pragma pack(pop)

void AudioPlayer::LoadAudioSnd() {
    static_assert(sizeof(SoundHeader_mm7) == 52, "Wrong type size");

    std::string file_path = MakeDataPath("sounds", "audio.snd");
    fAudioSnd.open(MakeDataPath("sounds", "audio.snd"));

    uint32_t uNumSoundHeaders {};
    fAudioSnd.readOrFail(&uNumSoundHeaders, sizeof(uNumSoundHeaders));
    for (uint32_t i = 0; i < uNumSoundHeaders; i++) {
        SoundHeader_mm7 header_mm7;
        fAudioSnd.readOrFail(&header_mm7, sizeof(header_mm7));
        SoundHeader header;
        header.uFileOffset = header_mm7.uFileOffset;
        header.uCompressedSize = header_mm7.uCompressedSize;
        header.uDecompressedSize = header_mm7.uDecompressedSize;
        mSoundHeaders[toLower(header_mm7.pSoundName)] = header;
    }
}

void AudioPlayer::Initialize() {
    currentMusicTrack = MUSIC_Invalid;
    uMasterVolume = 127;

    SetMasterVolume(engine->config->settings.SoundLevel.value());
    SetVoiceVolume(engine->config->settings.VoiceLevel.value());
    if (bPlayerReady) {
        SetMusicVolume(engine->config->settings.MusicLevel.value());
    }
    LoadAudioSnd();

    bPlayerReady = true;
}

void PlayLevelMusic() {
    unsigned int map_id = pMapStats->GetMapInfo(pCurrentMapName);
    if (map_id) {
        pAudioPlayer->MusicPlayTrack((MusicID)pMapStats->pInfos[map_id].uRedbookTrackID);
    }
}


bool AudioPlayer::FindSound(const std::string &pName, AudioPlayer::SoundHeader *header) {
    if (header == nullptr) {
        return false;
    }

    std::map<std::string, SoundHeader>::iterator it = mSoundHeaders.find(toLower(pName));
    if (it == mSoundHeaders.end()) {
        return false;
    }

    *header = it->second;

    return true;
}


Blob AudioPlayer::LoadSound(int uSoundID) {  // bit of a kludge (load sound by ID index) - plays some interesting files
    SoundHeader header = { 0 };

    if (uSoundID < 0 || uSoundID > mSoundHeaders.size())
        return {};

    // iterate through to get sound by int ID
    std::map<std::string, SoundHeader>::iterator it = mSoundHeaders.begin();
    std::advance(it, uSoundID);

    if (it == mSoundHeaders.end())
        return {};

    header = it->second;

    fAudioSnd.seek(header.uFileOffset);
    if (header.uCompressedSize >= header.uDecompressedSize) {
        header.uCompressedSize = header.uDecompressedSize;
        if (header.uDecompressedSize) {
            return Blob::read(fAudioSnd, header.uDecompressedSize);
        } else {
            logger->warning("Can't load sound file!");
            return Blob();
        }
    } else {
        return zlib::Uncompress(Blob::read(fAudioSnd, header.uCompressedSize), header.uDecompressedSize);
    }
}


Blob AudioPlayer::LoadSound(const std::string &pSoundName) {
    SoundHeader header = { 0 };
    if (!FindSound(pSoundName, &header)) {
        logger->warning("AudioPlayer: {} can't load sound header!", pSoundName);
        return Blob();
    }

    fAudioSnd.seek(header.uFileOffset);
    if (header.uCompressedSize >= header.uDecompressedSize) {
        header.uCompressedSize = header.uDecompressedSize;
        if (header.uDecompressedSize) {
            return Blob::read(fAudioSnd, header.uDecompressedSize);
        } else {
            logger->warning("AudioPlayer: {} can't load sound file!", pSoundName);
            return Blob();
        }
    } else {
        return zlib::Uncompress(Blob::read(fAudioSnd, header.uCompressedSize), header.uDecompressedSize);
    }
}

void AudioPlayer::playSpellSound(SPELL_TYPE spell, unsigned int pid, bool is_impact) {
    if (spell != SPELL_NONE)
        playSound(static_cast<SoundID>(SpellSoundIds[spell] + is_impact), pid, 0, -1, 0, 0);
}

