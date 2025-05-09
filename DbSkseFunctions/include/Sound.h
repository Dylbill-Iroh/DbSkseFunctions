#pragma once

RE::BGSMusicType* GetCurrentMusicType(RE::StaticFunctionTag*);

int GetNumberOfTracksInMusicType(RE::StaticFunctionTag*, RE::BGSMusicType* musicType);

int GetMusicTypeTrackIndex(RE::StaticFunctionTag*, RE::BGSMusicType * musicType);

void SetMusicTypeTrackIndex(RE::StaticFunctionTag*, RE::BGSMusicType * musicType, int index);

int GetMusicTypePriority(RE::StaticFunctionTag*, RE::BGSMusicType * musicType);

void SetMusicTypePriority(RE::StaticFunctionTag*, RE::BGSMusicType * musicType, int priority);

int GetMusicTypeStatus(RE::StaticFunctionTag*, RE::BGSMusicType * musicType);

void CreateSoundEvent(RE::TESForm* soundOrDescriptor, RE::BSSoundHandle& soundHandle, std::vector<RE::VMHandle> vmHandles, int intervalCheck);

RE::BSSoundHandle* GetSoundHandleById(int id);

int PlaySound(RE::StaticFunctionTag*, RE::TESSound * akSound, RE::TESObjectREFR * akSource, float volume, RE::TESForm * eventReceiverForm, RE::BGSBaseAlias * eventReceiverAlias, RE::ActiveEffect * eventReceiverActiveEffect);

int PlaySoundDescriptor(RE::StaticFunctionTag*, RE::BGSSoundDescriptorForm * akSoundDescriptor, RE::TESObjectREFR * akSource, float volume, RE::TESForm * eventReceiverForm, RE::BGSBaseAlias * eventReceiverAlias, RE::ActiveEffect * eventReceiverActiveEffect);

bool SetSoundInstanceSource(RE::StaticFunctionTag*, int instanceID, RE::TESObjectREFR * ref);

RE::BGSSoundCategory* GetParentSoundCategory(RE::StaticFunctionTag*, RE::BGSSoundCategory * akSoundCategory);

RE::BGSSoundCategory* GetSoundCategoryForSoundDescriptor(RE::StaticFunctionTag*, RE::BGSSoundDescriptorForm * akSoundDescriptor);

void SetSoundCategoryForSoundDescriptor(RE::StaticFunctionTag*, RE::BGSSoundDescriptorForm * akSoundDescriptor, RE::BGSSoundCategory * akSoundCategory);

float GetSoundCategoryVolume(RE::StaticFunctionTag*, RE::BGSSoundCategory * akCategory);

float GetSoundCategoryFrequency(RE::StaticFunctionTag*, RE::BGSSoundCategory * akCategory);


