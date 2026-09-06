#pragma once

// ============================================================================
// FORM TYPE TABLE
//
// One line per form type: (FormType enum name, RE class, Papyrus script type).
// Everything else in this file is generated from it, so adding a type means
// adding one row here -- not touching the switch, the registration, or the psc.
//
// Only types that exist as Papyrus script classes are listed. Adding a row for
// something Papyrus has no class for (Skill, TLOD, ...) would compile but could
// never be called.
// ============================================================================

#define DB_MASTER_TYPE_LIST                                                                \
    /* Native FormTypes (Underlying Enum Name, RE Class, Script Name) */                   \
    X(RE::FormType::Keyword,             RE::BGSKeyword,               Keyword)            \
    X(RE::FormType::LocationRefType,     RE::BGSLocationRefType,       LocationRefType)    \
    X(RE::FormType::Action,              RE::BGSAction,                Action)             \
    X(RE::FormType::TextureSet,          RE::BGSTextureSet,            TextureSet)         \
    X(RE::FormType::Global,              RE::TESGlobal,                GlobalVariable)     \
    X(RE::FormType::Class,               RE::TESClass,                 Class)              \
    X(RE::FormType::Faction,             RE::TESFaction,               Faction)            \
    X(RE::FormType::HeadPart,            RE::BGSHeadPart,              HeadPart)           \
    X(RE::FormType::Race,                RE::TESRace,                  Race)               \
    X(RE::FormType::Sound,               RE::TESSound,                 Sound)              \
    X(RE::FormType::AcousticSpace,       RE::BGSAcousticSpace,         AcousticSpace)      \
    X(RE::FormType::MagicEffect,         RE::EffectSetting,            MagicEffect)        \
    X(RE::FormType::LandTexture,         RE::TESLandTexture,           LandTexture)        \
    X(RE::FormType::Enchantment,         RE::EnchantmentItem,          Enchantment)        \
    X(RE::FormType::Spell,               RE::SpellItem,                Spell)              \
    X(RE::FormType::Scroll,              RE::ScrollItem,               Scroll)             \
    X(RE::FormType::Activator,           RE::TESObjectACTI,            Activator)          \
    X(RE::FormType::TalkingActivator,    RE::BGSTalkingActivator,      TalkingActivator)   \
    X(RE::FormType::Armor,               RE::TESObjectARMO,            Armor)              \
    X(RE::FormType::Book,                RE::TESObjectBOOK,            Book)               \
    X(RE::FormType::Container,           RE::TESObjectCONT,            Container)          \
    X(RE::FormType::Door,                RE::TESObjectDOOR,            Door)               \
    X(RE::FormType::Ingredient,          RE::IngredientItem,           Ingredient)         \
    X(RE::FormType::Light,               RE::TESObjectLIGH,            Light)              \
    X(RE::FormType::Misc,                RE::TESObjectMISC,            MiscObject)         \
    X(RE::FormType::Apparatus,           RE::BGSApparatus,             Apparatus)          \
    X(RE::FormType::Static,              RE::TESObjectSTAT,            Static)             \
    X(RE::FormType::MovableStatic,       RE::BGSMovableStatic,         MovableStatic)      \
    X(RE::FormType::Grass,               RE::TESGrass,                 Grass)              \
    X(RE::FormType::Tree,                RE::TESObjectTREE,            Tree)               \
    X(RE::FormType::Flora,               RE::TESFlora,                 Flora)              \
    X(RE::FormType::Furniture,           RE::TESFurniture,             Furniture)          \
    X(RE::FormType::Weapon,              RE::TESObjectWEAP,            Weapon)             \
    X(RE::FormType::Ammo,                RE::TESAmmo,                  Ammo)               \
    X(RE::FormType::NPC,                 RE::TESNPC,                   ActorBase)          \
    X(RE::FormType::LeveledNPC,          RE::TESLevCharacter,          LeveledActor)       \
    X(RE::FormType::KeyMaster,           RE::TESKey,                   Key)                \
    X(RE::FormType::AlchemyItem,         RE::AlchemyItem,              Potion)             \
    X(RE::FormType::IdleMarker,          RE::BGSIdleMarker,            IdleMarker)         \
    X(RE::FormType::Note,                RE::BGSNote,                  Note)               \
    X(RE::FormType::ConstructibleObject, RE::BGSConstructibleObject,   ConstructibleObject)\
    X(RE::FormType::Projectile,          RE::BGSProjectile,            Projectile)         \
    X(RE::FormType::Hazard,              RE::BGSHazard,                Hazard)             \
    X(RE::FormType::SoulGem,             RE::TESSoulGem,               SoulGem)            \
    X(RE::FormType::LeveledItem,         RE::TESLevItem,               LeveledItem)        \
    X(RE::FormType::Weather,             RE::TESWeather,               Weather)            \
    X(RE::FormType::Climate,             RE::TESClimate,               Climate)            \
    X(RE::FormType::ReferenceEffect,     RE::BGSReferenceEffect,       ReferenceEffect)    \
    X(RE::FormType::Region,              RE::TESRegion,                Region)             \
    X(RE::FormType::Cell,                RE::TESObjectCELL,            Cell)               \
    X(RE::FormType::Reference,           RE::TESObjectREFR,            ObjectReference)    \
    X(RE::FormType::ActorCharacter,      RE::Actor,                    Actor)              \
    X(RE::FormType::WorldSpace,          RE::TESWorldSpace,            WorldSpace)         \
    X(RE::FormType::Dialogue,            RE::TESTopic,                 Topic)              \
    X(RE::FormType::Info,                RE::TESTopicInfo,             TopicInfo)          \
    X(RE::FormType::Quest,               RE::TESQuest,                 Quest)              \
    X(RE::FormType::Idle,                RE::TESIdleForm,              Idle)               \
    X(RE::FormType::Package,             RE::TESPackage,               Package)            \
    X(RE::FormType::CombatStyle,         RE::TESCombatStyle,           CombatStyle)        \
    X(RE::FormType::LoadScreen,          RE::TESLoadScreen,            LoadScreen)         \
    X(RE::FormType::LeveledSpell,        RE::TESLevSpell,              LeveledSpell)       \
    X(RE::FormType::AnimatedObject,      RE::TESObjectANIO,            AnimatedObject)     \
    X(RE::FormType::Water,               RE::TESWaterForm,             Water)              \
    X(RE::FormType::EffectShader,        RE::TESEffectShader,          EffectShader)       \
    X(RE::FormType::Explosion,           RE::BGSExplosion,             Explosion)          \
    X(RE::FormType::Debris,              RE::BGSDebris,                Debris)             \
    X(RE::FormType::ImageSpace,          RE::TESImageSpace,            ImageSpace)         \
    X(RE::FormType::ImageAdapter,        RE::TESImageSpaceModifier,    ImageSpaceModifier) \
    X(RE::FormType::FormList,            RE::BGSListForm,              FormList)           \
    X(RE::FormType::Perk,                RE::BGSPerk,                  Perk)               \
    X(RE::FormType::BodyPartData,        RE::BGSBodyPartData,          BodyPartData)       \
    X(RE::FormType::AddonNode,           RE::BGSAddonNode,             AddonNode)          \
    X(RE::FormType::ActorValueInfo,      RE::ActorValueInfo,           ActorValueInfo)     \
    X(RE::FormType::CameraShot,          RE::BGSCameraShot,            CameraShot)         \
    X(RE::FormType::CameraPath,          RE::BGSCameraPath,            CameraPath)         \
    X(RE::FormType::VoiceType,           RE::BGSVoiceType,             VoiceType)          \
    X(RE::FormType::MaterialType,        RE::BGSMaterialType,          MaterialType)       \
    X(RE::FormType::Impact,              RE::BGSImpactData,            ImpactData)         \
    X(RE::FormType::ImpactDataSet,       RE::BGSImpactDataSet,         ImpactDataSet)      \
    X(RE::FormType::Armature,            RE::TESObjectARMA,            ArmorAddon)         \
    X(RE::FormType::EncounterZone,       RE::BGSEncounterZone,         EncounterZone)      \
    X(RE::FormType::Location,            RE::BGSLocation,              Location)           \
    X(RE::FormType::Message,             RE::BGSMessage,               Message)            \
    X(RE::FormType::LightingMaster,      RE::BGSLightingTemplate,      LightingTemplate)   \
    X(RE::FormType::MusicType,           RE::BGSMusicType,             MusicType)          \
    X(RE::FormType::Footstep,            RE::BGSFootstep,              Footstep)           \
    X(RE::FormType::FootstepSet,         RE::BGSFootstepSet,           FootstepSet)        \
    X(RE::FormType::DialogueBranch,      RE::BGSDialogueBranch,        DialogueBranch)     \
    X(RE::FormType::MusicTrack,          RE::BGSMusicTrackFormWrapper, MusicTrack)         \
    X(RE::FormType::WordOfPower,         RE::TESWordOfPower,           WordOfPower)        \
    X(RE::FormType::Shout,               RE::TESShout,                 Shout)              \
    X(RE::FormType::EquipSlot,           RE::BGSEquipSlot,             EquipSlot)          \
    X(RE::FormType::Relationship,        RE::BGSRelationship,          Relationship)       \
    X(RE::FormType::Scene,               RE::BGSScene,                 Scene)              \
    X(RE::FormType::AssociationType,     RE::BGSAssociationType,       AssociationType)    \
    X(RE::FormType::Outfit,              RE::BGSOutfit,                Outfit)             \
    X(RE::FormType::ArtObject,           RE::BGSArtObject,             Art)                \
    X(RE::FormType::MaterialObject,      RE::BGSMaterialObject,        MaterialObject)     \
    X(RE::FormType::MovementType,        RE::BGSMovementType,          MovementType)       \
    X(RE::FormType::SoundRecord,         RE::BGSSoundDescriptorForm,   SoundDescriptor)    \
    X(RE::FormType::DualCastData,        RE::BGSDualCastData,          DualCastData)       \
    X(RE::FormType::SoundCategory,       RE::BGSSoundCategory,         SoundCategory)      \
    X(RE::FormType::SoundOutputModel,    RE::BGSSoundOutput,           SoundOutputModel)   \
    X(RE::FormType::CollisionLayer,      RE::BGSCollisionLayer,        CollisionLayer)     \
    X(RE::FormType::ColorForm,           RE::BGSColorForm,             ColorForm)          \
    X(RE::FormType::ReverbParam,         RE::BGSReverbParameters,      ReverbParameters)   \
    X(RE::FormType::LensFlare,           RE::BGSLensFlare,             LensFlare)          \
    X(RE::FormType::VolumetricLighting,  RE::BGSVolumetricLighting,    VolumetricLighting) \
	X(200,                				 RE::BGSBaseAlias,             Alias)              \
	X(201,                				 RE::BGSRefAlias,              ReferenceAlias)     \
	X(202,                				 RE::BGSLocAlias,              LocationAlias)      \
	X(203,                				 RE::ActiveEffect,             ActiveMagicEffect)

namespace FormTypeTable {
    enum class ExtendedFormType : std::int32_t // Force int32_t to safely hold 200+ and RE::FormType
    {
        #define X(Id, Class, ScriptName) ScriptName = static_cast<std::int32_t>(Id),
        DB_MASTER_TYPE_LIST
        #undef X
    };
}