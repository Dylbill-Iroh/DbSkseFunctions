#include <thread>
#include "Sound.h"
#include "GeneralFunctions.h"

namespace logger = SKSE::log;

//Music=================================================================================================================================================================

//function copied from More Informative Console
RE::BGSMusicType* GetCurrentMusicType(RE::StaticFunctionTag*)
{
    RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
    RE::BGSMusicType* currentPriorityType = nullptr;

    if (dataHandler) {
        RE::BSTArray<RE::TESForm*>* musicTypeArray = &(dataHandler->GetFormArray(RE::FormType::MusicType));

        RE::BSIMusicTrack* currentPriorityTrack = nullptr;
        std::int8_t currentPriority = 127;
        int ic = 0;

        for (RE::BSTArray<RE::TESForm*>::iterator itr = musicTypeArray->begin(); itr != musicTypeArray->end() && ic < musicTypeArray->size(); itr++, ic++) {
            RE::TESForm* baseForm = *itr;
            
            if (gfuncs::IsFormValid(baseForm)) {
                RE::BGSMusicType* musicType = static_cast<RE::BGSMusicType*>(baseForm);
                RE::BSIMusicType::MUSIC_STATUS musicStatus = musicType->typeStatus.get();

                if (musicStatus == RE::BSIMusicType::MUSIC_STATUS::kPlaying) {
                    uint32_t currentTrackIndex = musicType->currentTrackIndex;

                    if (musicType->tracks.size() > currentTrackIndex) {
                        RE::BSIMusicTrack* currentTrack = musicType->tracks[currentTrackIndex];

                        //if the track takes priority of the current priority track we found
                        if (currentTrack && currentPriority > musicType->priority) {
                            currentPriorityTrack = currentTrack;
                            currentPriorityType = musicType;
                            currentPriority = musicType->priority;
                        }
                    }
                }
            }
        }
    }
    return currentPriorityType;
}

int GetNumberOfTracksInMusicType(RE::StaticFunctionTag*, RE::BGSMusicType* musicType) {
    if (!gfuncs::IsFormValid(musicType)) {
        logger::warn("{}: musicType doesn't exist or isn't valid", __func__);
        return false;
    }
    return musicType->tracks.size();
}

int GetMusicTypeTrackIndex(RE::StaticFunctionTag*, RE::BGSMusicType* musicType) {
    if (!gfuncs::IsFormValid(musicType)) {
        logger::warn("{}: musicType doesn't exist or isn't valid", __func__);
        return false;
    }
    return musicType->currentTrackIndex;
}

void SetMusicTypeTrackIndex(RE::StaticFunctionTag*, RE::BGSMusicType* musicType, int index) {
    if (!gfuncs::IsFormValid(musicType)) {
        logger::warn("{}: musicType doesn't exist or isn't valid", __func__);
        return;
    }
    if (index >= musicType->tracks.size()) {
        index = musicType->tracks.size() - 1;
    }
    else if (index < 0) {
        index = 0;
    }

    if (musicType == GetCurrentMusicType(nullptr)) {
        musicType->tracks[GetMusicTypeTrackIndex(nullptr, musicType)]->DoFinish(false, 3.0);
        musicType->currentTrackIndex = index;
        musicType->tracks[index]->DoPlay();
    }
    else {
        musicType->currentTrackIndex = index;
    }
}

int GetMusicTypePriority(RE::StaticFunctionTag*, RE::BGSMusicType* musicType) {
    if (!gfuncs::IsFormValid(musicType)) {
        logger::warn("{}: musicType doesn't exist or isn't valid", __func__);
        return -1;
    }
    return musicType->priority;
}

void SetMusicTypePriority(RE::StaticFunctionTag*, RE::BGSMusicType* musicType, int priority) {
    if (!gfuncs::IsFormValid(musicType)) {
        logger::warn("{}: musicType doesn't exist or isn't valid", __func__);
        return;
    }
    musicType->priority = priority;
}

int GetMusicTypeStatus(RE::StaticFunctionTag*, RE::BGSMusicType* musicType) {
    if (!gfuncs::IsFormValid(musicType)) {
        logger::warn("{}: musicType doesn't exist or isn't valid", __func__);
        return -1;
    }
    return musicType->typeStatus.underlying();
}

//Sound =====================================================================================================================================================

std::map<int, RE::BSSoundHandle> playedSoundHandlesMap;
RE::BSFixedString soundFinishEvent = "OnSoundFinish";

void CreateSoundEvent(RE::TESForm* soundOrDescriptor, RE::BSSoundHandle& soundHandle, std::vector<RE::VMHandle> vmHandles, int intervalCheck) {
    std::thread t([=]() {
        playedSoundHandlesMap[int(soundHandle.soundID)] = soundHandle;
        //wait for sound to finish playing, then send events for handles. state 2 == stopped
        while (soundHandle.state.underlying() != 2 && soundHandle.IsValid()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(intervalCheck));
        }

        //remove sound handle from playedSoundHandlesMap
        auto it = playedSoundHandlesMap.find(soundHandle.soundID);
        if (it != playedSoundHandlesMap.end()) {
            playedSoundHandlesMap.erase(it);
        }

        auto* args = RE::MakeFunctionArguments((RE::TESForm*)soundOrDescriptor, (int)soundHandle.soundID);
        gfuncs::SendEvents(vmHandles, soundFinishEvent, args);
        });
    t.detach();
}

RE::BSSoundHandle* GetSoundHandleById(int id) {
    auto it = playedSoundHandlesMap.find(id);
    if (it != playedSoundHandlesMap.end()) {
        if (it->second.soundID == id) {
            return &it->second;
        }
    }

    return nullptr;
}

int PlaySound(RE::StaticFunctionTag*, RE::TESSound* akSound, RE::TESObjectREFR* akSource, float volume,
    RE::TESForm* eventReceiverForm, RE::BGSBaseAlias* eventReceiverAlias, RE::ActiveEffect* eventReceiverActiveEffect) {

    logger::trace("{} called", __func__);

    if (!gfuncs::IsFormValid(akSound)) {
        logger::warn("{}: error, akSound doesn't exist", __func__);
        return -1;
    }

    if (!gfuncs::IsFormValid(akSource)) {
        logger::warn("{}: error, akSource doesn't exist", __func__);
        return -1;
    }

    auto* audiomanager = RE::BSAudioManager::GetSingleton();
    RE::BSSoundHandle soundHandle;

    audiomanager->BuildSoundDataFromDescriptor(soundHandle, akSound->descriptor->soundDescriptor);

    soundHandle.SetObjectToFollow(akSource->Get3D());
    soundHandle.SetVolume(volume);
    soundHandle.Play();

    std::vector<RE::VMHandle> vmHandles;

    if (gfuncs::IsFormValid(eventReceiverForm)) {
        RE::VMHandle vmHandle = gfuncs::GetHandle(eventReceiverForm);
        if (vmHandle != NULL) {
            vmHandles.push_back(vmHandle);
        }
    }

    if (eventReceiverAlias) {
        RE::VMHandle vmHandle = gfuncs::GetHandle(eventReceiverAlias);
        if (vmHandle != NULL) {
            vmHandles.push_back(vmHandle);
        }
    }

    if (eventReceiverActiveEffect) {
        RE::VMHandle vmHandle = gfuncs::GetHandle(eventReceiverActiveEffect);
        if (vmHandle != NULL) {
            vmHandles.push_back(vmHandle);
        }
    }

    CreateSoundEvent(akSound, soundHandle, vmHandles, 1000);
    return soundHandle.soundID;
}

int PlaySoundDescriptor(RE::StaticFunctionTag*, RE::BGSSoundDescriptorForm* akSoundDescriptor, RE::TESObjectREFR* akSource, float volume,
    RE::TESForm* eventReceiverForm, RE::BGSBaseAlias* eventReceiverAlias, RE::ActiveEffect* eventReceiverActiveEffect) {

    logger::trace("{} called", __func__);

    if (!gfuncs::IsFormValid(akSoundDescriptor)) {
        logger::warn("{}: error, akSoundDescriptor doesn't exist", __func__);
        return -1;
    }

    if (!gfuncs::IsFormValid(akSource)) {
        logger::warn("{}: error, akSource doesn't exist", __func__);
        return -1;
    }

    auto* audiomanager = RE::BSAudioManager::GetSingleton();
    RE::BSSoundHandle soundHandle;

    audiomanager->BuildSoundDataFromDescriptor(soundHandle, akSoundDescriptor->soundDescriptor);

    soundHandle.SetObjectToFollow(akSource->Get3D());
    soundHandle.SetVolume(volume);
    soundHandle.Play();

    std::vector<RE::VMHandle> vmHandles;

    if (gfuncs::IsFormValid(eventReceiverForm)) {
        RE::VMHandle vmHandle = gfuncs::GetHandle(eventReceiverForm);
        if (vmHandle != NULL) {
            vmHandles.push_back(vmHandle);
        }
    }

    if (eventReceiverAlias) {
        RE::VMHandle vmHandle = gfuncs::GetHandle(eventReceiverAlias);
        if (vmHandle != NULL) {
            vmHandles.push_back(vmHandle);
        }
    }

    if (eventReceiverActiveEffect) {
        RE::VMHandle vmHandle = gfuncs::GetHandle(eventReceiverActiveEffect);
        if (vmHandle != NULL) {
            vmHandles.push_back(vmHandle);
        }
    }

    CreateSoundEvent(akSoundDescriptor, soundHandle, vmHandles, 1000);
    return soundHandle.soundID;
}

bool SetSoundInstanceSource(RE::StaticFunctionTag*, int instanceID, RE::TESObjectREFR* ref) {
    if (!gfuncs::IsFormValid(ref)) {
        logger::warn("{} error: ref doesn't exist or isn't valid.", __func__);
        return false;
    }

    RE::BSSoundHandle* soundHandle = GetSoundHandleById(instanceID);
    if (!soundHandle) {
        logger::warn("{} error: couldn't find sound handle for instanceID [{}]", __func__, instanceID);
        return false;
    }

    if (soundHandle->state.underlying() != 2 && soundHandle->IsValid()) {
        RE::NiAVObject* obj3d = gfuncs::GetNiAVObjectForRef(ref);
        if (!obj3d) {
            logger::warn("{} error: couldn't get 3d for ref: [{}]", __func__, gfuncs::GetFormName(ref));
            return false;
        }
        else {
            soundHandle->SetObjectToFollow(obj3d);
            logger::debug("{}: instanceID [{}] set to follow ref: [{}]", __func__, instanceID, gfuncs::GetFormName(ref));
            return true;
        }
    }

    logger::warn("{} error: instanceID [{}] is no longer playing or valid. Not set to follow ref: [{}]", __func__, instanceID, gfuncs::GetFormName(ref));
    return false;
}

RE::BGSSoundCategory* GetParentSoundCategory(RE::StaticFunctionTag*, RE::BGSSoundCategory* akSoundCategory) {
    if (!gfuncs::IsFormValid(akSoundCategory)) {
        logger::warn("{}: error, akSoundCategory doesn't exist", __func__);
        return nullptr;
    }

    return akSoundCategory->parentCategory;
}

RE::BGSSoundCategory* GetSoundCategoryForSoundDescriptor(RE::StaticFunctionTag*, RE::BGSSoundDescriptorForm* akSoundDescriptor) {
    if (!gfuncs::IsFormValid(akSoundDescriptor)) {
        logger::warn("{}: error, akSoundDescriptor doesn't exist", __func__);
        return nullptr;
    }

    if (!akSoundDescriptor->soundDescriptor) {
        logger::warn("{}: error, akSoundDescriptor->soundDescriptor doesn't exist", __func__);
        return nullptr;
    }

    return akSoundDescriptor->soundDescriptor->category;
}

void SetSoundCategoryForSoundDescriptor(RE::StaticFunctionTag*, RE::BGSSoundDescriptorForm* akSoundDescriptor, RE::BGSSoundCategory* akSoundCategory) {
    if (!gfuncs::IsFormValid(akSoundCategory)) {
        logger::warn("{}: error, akSoundCategory doesn't exist", __func__);
        return;
    }

    if (!gfuncs::IsFormValid(akSoundDescriptor)) {
        logger::warn("{}: error, akSoundDescriptor doesn't exist", __func__);
        return;
    }

    if (!akSoundDescriptor->soundDescriptor) {
        logger::error("{}: error, akSoundDescriptor->soundDescriptor doesn't exist", __func__);
        return;
    }

    akSoundDescriptor->soundDescriptor->category = akSoundCategory;
}

float GetSoundCategoryVolume(RE::StaticFunctionTag*, RE::BGSSoundCategory* akCategory) {
    if (!gfuncs::IsFormValid(akCategory)) {
        logger::warn("{}: error, akCategory doesn't exist", __func__);
        return -1.0;
    }

    return akCategory->GetCategoryVolume();
}

float GetSoundCategoryFrequency(RE::StaticFunctionTag*, RE::BGSSoundCategory* akCategory) {
    if (!gfuncs::IsFormValid(akCategory)) {
        logger::warn("{}: error, akCategory doesn't exist", __func__);
        return -1.0;
    }

    return akCategory->GetCategoryFrequency();
}
