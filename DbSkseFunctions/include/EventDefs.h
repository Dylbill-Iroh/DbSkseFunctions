#pragma once

//some events in the commonlibSSE-NG library have missing definitions. I've put a few here thanks to skymp
//https://github.com/skyrim-multiplayer/skymp/blob/e4d1de332c788cc512ab26d56740035eee676df6/skyrim-platform/src/platform_se/skyrim_platform/game/Events.h#L4

namespace RE {
    struct TESPerkEntryRunEvent {
        RE::NiPointer<RE::TESObjectREFR> target;
        RE::NiPointer<RE::TESObjectREFR> owner;
        RE::FormID perkId;
        uint32_t flag;
    };

    struct TESTriggerEvent
    {
        RE::NiPointer<RE::TESObjectREFR> target;
        RE::NiPointer<RE::TESObjectREFR> caster;
    };

    struct TESTriggerEnterEvent
    {
        RE::NiPointer<RE::TESObjectREFR> target;
        RE::NiPointer<RE::TESObjectREFR> caster;
    };

    struct TESTriggerLeaveEvent
    {
        RE::NiPointer<RE::TESObjectREFR> target;
        RE::NiPointer<RE::TESObjectREFR> caster;
    };

    struct TESSellEvent
    {
        RE::NiPointer<RE::TESObjectREFR> target;
        RE::NiPointer<RE::TESObjectREFR> seller;
    };

    struct TESPackageEvent
    {
        enum class EventType : uint32_t // not sure
        {
            kStart = 0,
            kChange = 1,
            kEnd = 2
        };
        RE::NiPointer<RE::TESObjectREFR> actor;
        RE::FormID package;
        EventType type;
    };

    struct TESDestructionStageChangedEvent
    {
        RE::NiPointer<RE::TESObjectREFR> target;
        uint32_t oldStage;
        uint32_t newStage;
    };

    struct TESObjectREFRTranslationEvent
    {
        enum class EventType : uint32_t
        {
            kFailed = 0,
            kCompleted = 1,
            kAlmostCompleted = 2
        };
        RE::NiPointer<RE::TESObjectREFR> refr;
        EventType type;
    };
}