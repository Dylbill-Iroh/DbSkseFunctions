#include "ProjectileFunctions.h"
#include "GeneralFunctions.h"

namespace logger = SKSE::log;

std::map<RE::TESObjectREFR*, std::vector<TrackedProjectileData>> recentHitProjectiles;
std::map<RE::TESObjectREFR*, std::vector<TrackedProjectileData>> recentShotProjectiles;

//general projectile functions==========================================================================================================================

bool DidShooterHitRefWithProjectile(RE::TESObjectREFR* shooter, RE::TESObjectREFR* ref, TrackedProjectileData& data) {
    return (shooter == data.shooter && ref == data.target);
}

bool DidProjectileHitRef(RE::Projectile* akProjectile, RE::TESObjectREFR* ref) {
    if (gfuncs::IsFormValid(akProjectile) && gfuncs::IsFormValid(ref)) {

        //logger::trace("akProjectile[{}] found", akProjectile->GetDisplayFullName());
        auto impacts = akProjectile->GetProjectileRuntimeData().impacts;
        if (!impacts.empty()) {
            for (auto* impactData : impacts) {
                if (impactData) {
                    auto hitRef = impactData->collidee;
                    if (hitRef) {
                        //logger::trace("hitRef found");
                        auto hitRefPtr = hitRef.get();
                        if (hitRefPtr) {
                            //logger::trace("hitRefPtr found");
                            auto* hitRefRef = hitRefPtr.get();
                            if (gfuncs::IsFormValid(hitRefRef)) {
                                return (hitRefRef == ref);
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool DidRefShootProjectile(RE::Projectile* akProjectile, RE::TESObjectREFR* ref) {
    if (gfuncs::IsFormValid(akProjectile) && gfuncs::IsFormValid(ref)) {
        auto shooterHandle = akProjectile->GetProjectileRuntimeData().shooter;
        if (shooterHandle) {
            auto shooterRefPtr = shooterHandle.get();
            if (shooterRefPtr) {
                auto* shooter = shooterRefPtr.get();
                if (gfuncs::IsFormValid(shooter)) {
                    return (shooter == ref);
                }
            }
        }
    }
    return false;
}

bool DidProjectileHitRefWithAmmoFromShooter(RE::TESObjectREFR* shooter, RE::TESObjectREFR* ref, RE::TESAmmo* akAmmo, TrackedProjectileData& data) {
    return (shooter == data.shooter && ref == data.target && akAmmo == data.ammo);
}

bool DidProjectileHitRefWithAmmoFromShooter(RE::TESObjectREFR* shooter, RE::TESObjectREFR* ref, RE::Projectile* akProjectile, RE::TESAmmo* akAmmo) {
    if (gfuncs::IsFormValid(shooter) && gfuncs::IsFormValid(ref) && gfuncs::IsFormValid(akProjectile) && gfuncs::IsFormValid(akAmmo)) {
        auto& runtimeData = akProjectile->GetProjectileRuntimeData();

        if (runtimeData.ammoSource != akAmmo) {
            return false;
        }

        RE::TESObjectREFR* projectileShooter = nullptr;

        if (runtimeData.shooter) {
            auto projectileShooterPtr = runtimeData.shooter.get();
            if (projectileShooterPtr) {
                projectileShooter = projectileShooterPtr.get();
            }
        }

        if (gfuncs::IsFormValid(projectileShooter)) {
            if (projectileShooter != shooter) {
                return false;
            }
        }

        auto impacts = runtimeData.impacts;
        if (!impacts.empty()) {
            for (auto* impactData : impacts) {
                if (impactData) {
                    //logger::trace("impactData found");
                    auto hitRef = impactData->collidee;
                    if (hitRef) {
                        //logger::trace("hitRef found");
                        auto hitRefPtr = hitRef.get();
                        if (hitRefPtr) {
                            //logger::trace("hitRefPtr found");
                            auto* hitRefRef = hitRefPtr.get();
                            if (gfuncs::IsFormValid(hitRefRef)) {
                                return (hitRefRef == ref);
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

//projectile papyrus functions ==================================================================================================

int GetProjectileType(RE::BGSProjectile* projectile) {
    if (!gfuncs::IsFormValid(projectile)) {
        return 0;
    }

    if (projectile->IsMissile()) {
        return 1;
    }
    else if (projectile->IsGrenade()) {
        return 2;
    }
    else if (projectile->IsBeam()) {
        return 3;
    }
    else if (projectile->IsFlamethrower()) {
        return 4;
    }
    else if (projectile->IsCone()) {
        return 5;
    }
    else if (projectile->IsBarrier()) {
        return 6;
    }
    else if (projectile->IsArrow()) {
        return 7;
    }
    return 0;
}

RE::BGSTextureSet* GetProjectileBaseDecal(RE::StaticFunctionTag*, RE::BGSProjectile* projectile) {
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("projectile doesn't exist or isn't valid");
        return nullptr;
    }

    if (gfuncs::IsFormValid(projectile->data.decalData)) {
        return projectile->data.decalData;
    }
    return nullptr;
}

bool SetProjectileBaseDecal(RE::StaticFunctionTag*, RE::BGSProjectile* projectile, RE::BGSTextureSet* decalTextureSet) {
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("projectile doesn't exist or isn't valid");
        return false;
    }

    projectile->data.decalData = decalTextureSet;

    return (projectile->data.decalData == decalTextureSet);
}

RE::BGSExplosion* GetProjectileBaseExplosion(RE::StaticFunctionTag*, RE::BGSProjectile* projectile) {
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("projectile doesn't exist or isn't valid");
        return nullptr;
    }

    if (gfuncs::IsFormValid(projectile->data.explosionType)) {
        return projectile->data.explosionType;
    }
    return nullptr;
}

bool SetProjectileBaseExplosion(RE::StaticFunctionTag*, RE::BGSProjectile* projectile, RE::BGSExplosion* akExplosion) {
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("projectile doesn't exist or isn't valid");
        return false;
    }

    projectile->data.explosionType = akExplosion;

    return (projectile->data.explosionType == akExplosion);
}

float GetProjectileBaseCollisionRadius(RE::StaticFunctionTag*, RE::BGSProjectile* projectile) {
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("projectile doesn't exist or isn't valid");
        return 0.0;
    }

    return projectile->data.collisionRadius;
}

bool SetProjectileBaseCollisionRadius(RE::StaticFunctionTag*, RE::BGSProjectile* projectile, float radius) {
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("projectile doesn't exist or isn't valid");
        return false;
    }

    projectile->data.collisionRadius = radius;

    return (projectile->data.collisionRadius == radius);
}

float GetProjectileBaseCollisionConeSpread(RE::StaticFunctionTag*, RE::BGSProjectile* projectile) {
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("projectile doesn't exist or isn't valid");
        return 0.0;
    }

    return projectile->data.coneSpread;
}

bool SetProjectileBaseCollisionConeSpread(RE::StaticFunctionTag*, RE::BGSProjectile* projectile, float coneSpread) {
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("projectile doesn't exist or isn't valid");
        return false;
    }

    projectile->data.coneSpread = coneSpread;

    return (projectile->data.coneSpread == coneSpread);
}


int GetProjectileRefType(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    if (!gfuncs::IsFormValid(projectileRef)) {
        return 0;
    }

    auto* akProjectile = projectileRef->AsProjectile();

    if (!gfuncs::IsFormValid(akProjectile)) {
        return 0;
    }

    RE::BGSProjectile* projectileBase = akProjectile->GetProjectileBase();
    return GetProjectileType(projectileBase);
}

RE::TESObjectREFR* GetProjectileHitRef(RE::Projectile::ImpactData* impactData) {
    if (impactData) {
        if (impactData->collidee) {
            auto hitRefHandle = impactData->collidee;
            if (hitRefHandle) {
                auto hitRefPtr = hitRefHandle.get();
                if (hitRefPtr) {
                    return hitRefPtr.get();
                }
            }
        }
    }
    return nullptr;
}

std::vector<RE::TESObjectREFR*> GetProjectileHitRefs(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    std::vector<RE::TESObjectREFR*> hitRefs;
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("projectileRef doesn't exist or isn't valid");
        return hitRefs;
    }

    RE::Projectile* projectile = projectileRef->AsProjectile();
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("projectileRef[{}] is not a projectile", gfuncs::GetFormName(projectileRef));
        return hitRefs;
    }

    auto impacts = projectile->GetProjectileRuntimeData().impacts;
    if (!impacts.empty()) {
        for (auto* impactData : impacts) {
            auto* hitRef = GetProjectileHitRef(impactData);
            if (gfuncs::IsFormValid(hitRef)) {
                hitRefs.push_back(hitRef);
            }
        }
    }
    return hitRefs;
}

RE::TESObjectREFR* GetLastProjectileHitRefFromRuntimeData(RE::Projectile::PROJECTILE_RUNTIME_DATA& runtimeData) {
    RE::TESObjectREFR* hitRef;
    if (!runtimeData.impacts.empty()) {
        for (auto* impactData : runtimeData.impacts) {
            if (impactData) {
                RE::TESObjectREFR* akHitRef = GetProjectileHitRef(impactData);
                if (gfuncs::IsFormValid(akHitRef)) {
                    hitRef = akHitRef;
                }
            }
        }
    }
    return hitRef;
}

RE::TESObjectREFR* GetProjectileShooterFromRuntimeData(RE::Projectile::PROJECTILE_RUNTIME_DATA& runtimeData) {
    if (runtimeData.shooter) {
        auto refPtr = runtimeData.shooter.get();
        if (refPtr) {
            return refPtr.get();
        }
    }
    return nullptr;
}

RE::TESObjectREFR* GetProjectileShooter(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("projectileRef doesn't exist or isn't valid");
        return nullptr;
    }

    RE::Projectile* projectile = projectileRef->AsProjectile();
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("projectileRef[{}] is not a projectile", gfuncs::GetFormName(projectileRef));
        return nullptr;
    }

    return GetProjectileShooterFromRuntimeData(projectile->GetProjectileRuntimeData());
}

RE::BGSExplosion* GetProjectileExplosion(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("projectileRef doesn't exist or isn't valid");
        return nullptr;
    }

    RE::Projectile* projectile = projectileRef->AsProjectile();
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("projectileRef[{}] is not a projectile", gfuncs::GetFormName(projectileRef));
        return nullptr;
    }

    return projectile->GetProjectileRuntimeData().explosion;
}

RE::TESAmmo* GetProjectileAmmoSource(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("projectileRef doesn't exist or isn't valid");
        return nullptr;
    }

    RE::Projectile* projectile = projectileRef->AsProjectile();
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("projectileRef[{}] is not a projectile", gfuncs::GetFormName(projectileRef));
        return nullptr;
    }

    return projectile->GetProjectileRuntimeData().ammoSource;
}

RE::AlchemyItem* GetProjectilePoison(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    RE::AlchemyItem* poison;
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("projectileRef doesn't exist or isn't valid");
        return nullptr;
    }

    auto* arrowProjectile = projectileRef->As<RE::ArrowProjectile>();
    if (gfuncs::IsFormValid(arrowProjectile)) {
        poison = arrowProjectile->GetArrowRuntimeData().poison;
    }
    return poison;
}

RE::EnchantmentItem* GetProjectileEnchantment(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    RE::EnchantmentItem* enchantment;

    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("projectileRef doesn't exist or isn't valid");
        return nullptr;
    }

    auto* arrowProjectile = projectileRef->As<RE::ArrowProjectile>();
    if (gfuncs::IsFormValid(arrowProjectile)) {
        enchantment = arrowProjectile->GetArrowRuntimeData().enchantItem;
    }
    return enchantment;
}

RE::TESForm* GetProjectileMagicSource(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("projectileRef doesn't exist or isn't valid");
        return nullptr;
    }

    RE::Projectile* projectile = projectileRef->AsProjectile();
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("projectileRef[{}] is not a projectile", gfuncs::GetFormName(projectileRef));
        return nullptr;
    }

    return projectile->GetProjectileRuntimeData().spell;
}

RE::TESObjectWEAP* GetProjectileWeaponSource(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("projectileRef doesn't exist or isn't valid");
        return nullptr;
    }

    RE::Projectile* projectile = projectileRef->AsProjectile();
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("projectileRef[{}] is not a projectile", gfuncs::GetFormName(projectileRef));
        return nullptr;
    }

    return projectile->GetProjectileRuntimeData().weaponSource;
}

float GetProjectileWeaponDamage(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("projectileRef doesn't exist or isn't valid");
        return 0.0;
    }

    RE::Projectile* projectile = projectileRef->AsProjectile();
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("projectileRef[{}] is not a projectile", gfuncs::GetFormName(projectileRef));
        return 0.0;
    }

    return projectile->GetProjectileRuntimeData().weaponDamage;
}

float GetProjectilePower(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("projectileRef doesn't exist or isn't valid");
        return 0.0;
    }

    RE::Projectile* projectile = projectileRef->AsProjectile();
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("projectileRef[{}] is not a projectile", gfuncs::GetFormName(projectileRef));
        return 0.0;
    }

    return projectile->GetProjectileRuntimeData().power;
}

float GetProjectileDistanceTraveled(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("projectileRef doesn't exist or isn't valid");
        return 0.0;
    }

    RE::Projectile* projectile = projectileRef->AsProjectile();
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("projectileRef[{}] is not a projectile", gfuncs::GetFormName(projectileRef));
        return 0.0;
    }

    return projectile->GetProjectileRuntimeData().distanceMoved;
}

int GetProjectileImpactResult(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    int impactResult = 0;
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("projectileRef isn't valid or doesn't exist");
        return impactResult;
    }

    auto* missileProjectile = projectileRef->As<RE::MissileProjectile>();

    if (gfuncs::IsFormValid(missileProjectile)) {
        impactResult = static_cast<int>(missileProjectile->GetMissileRuntimeData().impactResult);
    }

    if (impactResult == 0) {
        if (gfuncs::IsFormValid(projectileRef)) { //check this again to be sure
            auto* coneProjectile = projectileRef->As<RE::ConeProjectile>();
            if (gfuncs::IsFormValid(coneProjectile)) {
                impactResult = static_cast<int>(coneProjectile->GetConeRuntimeData().impactResult);
            }
        }
    }
    return impactResult;
}

std::vector<int> GetProjectileCollidedLayers(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    std::vector<int> layers;
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("projectileRef doesn't exist or isn't valid");
        return layers;
    }

    RE::Projectile* projectile = projectileRef->AsProjectile();
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("projectileRef[{}] is not a projectile", gfuncs::GetFormName(projectileRef));
        return layers;
    }

    auto impacts = projectile->GetProjectileRuntimeData().impacts;
    if (!impacts.empty()) {
        for (auto* impactData : impacts) {
            if (impactData) {
                int collidedLayer = impactData->collidedLayer.underlying();
                layers.push_back(collidedLayer);
            }
        }
    }
    return layers;
}

std::string GetCollisionLayerName(RE::StaticFunctionTag*, int layer) {
    switch (layer) {
    case 1:
        return "Static";
    case 2:
        return "AnimStatic";
    case 3:
        return "Transparent";
    case 4:
        return "Clutter";
    case 5:
        return "Weapon";
    case 6:
        return "Projectile";
    case 7:
        return "Spell";
    case 8:
        return "Biped";
    case 9:
        return "Trees";
    case 10:
        return "Props";
    case 11:
        return "Water";
    case 12:
        return "Trigger";
    case 13:
        return "Terrain";
    case 14:
        return "Trap";
    case 15:
        return "NonCollidable";
    case 16:
        return "CloudTrap";
    case 17:
        return "Ground";
    case 18:
        return "Portal";
    case 19:
        return "DebrisSmall";
    case 20:
        return "DebrisLarge";
    case 21:
        return "AcousticSpace";
    case 22:
        return "ActorZone";
    case 23:
        return "ProjectileZone";
    case 24:
        return "GasTrap";
    case 25:
        return "ShellCasting";
    case 26:
        return "TransparentWall";
    case 27:
        return "InvisibleWall";
    case 28:
        return "TransparentSmallAnim";
    case 29:
        return "ClutterLarge";
    case 30:
        return "CharController";
    case 31:
        return "StairHelper";
    case 32:
        return "DeadBip";
    case 33:
        return "BipedNoCC";
    case 34:
        return "AvoidBox";
    case 35:
        return "CollisionBox";
    case 36:
        return "CameraSphere";
    case 37:
        return "DoorDetection";
    case 38:
        return "ConeProjectile";
    case 39:
        return "Camera";
    case 40:
        return "ItemPicker";
    case 41:
        return "LOS";
    case 42:
        return "PathingPick";
    case 43:
        return "Unused0";
    case 44:
        return "Unused1";
    case 45:
        return "SpellExplosion";
    case 46:
        return "DroppingPick";
    default:
        return "Unidentified";
    }
}

std::vector<std::string> GetProjectileCollidedLayerNames(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    std::vector<std::string> layers;
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("projectileRef doesn't exist or isn't valid");
        return layers;
    }

    std::vector<int> intLayers = GetProjectileCollidedLayers(nullptr, projectileRef);
    for (auto& layer : intLayers) {
        layers.push_back(GetCollisionLayerName(nullptr, layer));
    }

    return layers;
}

std::string GetProjectileNodeHitName(RE::Projectile::ImpactData* impactData) {
    RE::BSFixedString hitPartNodeName;
    if (impactData) {
        RE::NiNode* hitPart = impactData->damageRootNode;
        if (hitPart) {
            RE::BSFixedString hitPartNodeName = hitPart->name;
            if (hitPart->parent) {
                if (hitPart->parent->name == "SHIELD" || hitPartNodeName == "") {
                    hitPartNodeName = hitPart->parent->name;
                }
            }
        }
    }
    return hitPartNodeName.data();
}

std::vector<std::string> GetProjectileNodeHitNames(RE::StaticFunctionTag*, RE::TESObjectREFR* projectileRef) {
    std::vector<std::string> nodeNames;
    if (!gfuncs::IsFormValid(projectileRef)) {
        logger::warn("projectileRef doesn't exist or isn't valid");
        return nodeNames;
    }

    RE::Projectile* projectile = projectileRef->AsProjectile();
    if (!gfuncs::IsFormValid(projectile)) {
        logger::warn("projectileRef[{}] is not a projectile", gfuncs::GetFormName(projectileRef));
        return nodeNames;
    }

    auto impacts = projectile->GetProjectileRuntimeData().impacts;
    if (!impacts.empty()) {
        for (auto* impactData : impacts) {
            if (impactData) {
                std::string nodeName = GetProjectileNodeHitName(impactData);
                if (nodeName != "") {
                    nodeNames.push_back(nodeName);
                }
            }
        }
    }
    return nodeNames;
}

std::vector<RE::TESObjectREFR*> GetRecentProjectileHitRefs(RE::StaticFunctionTag*, RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled, int projectileType) {
    std::vector<RE::TESObjectREFR*> refs;
    if (!gfuncs::IsFormValid(ref)) {
        logger::warn("ref doesn't exist");
        return refs;
    }

    auto it = recentHitProjectiles.find(ref);
    if (it != recentHitProjectiles.end()) {
        if (it->second.size() == 0) {
            return refs;
        }
        if (projectileType >= 1 && projectileType <= 7) {
            for (auto& data : it->second) {
                if (GetProjectileRefType(nullptr, data.projectile) == projectileType) {
                    if ((data.projectile->Is3DLoaded() || !only3dLoaded) && (!data.projectile->IsDisabled() || !onlyEnabled)) {
                        refs.push_back(data.projectile);
                    }
                }
            }
        }
        else {
            for (auto& data : it->second) {
                if (gfuncs::IsFormValid(data.projectile)) {
                    if ((data.projectile->Is3DLoaded() || !only3dLoaded) && (!data.projectile->IsDisabled() || !onlyEnabled)) {
                        refs.push_back(data.projectile);
                    }
                }
            }
        }
    }
    return refs;
}

RE::TESObjectREFR* GetLastProjectileHitRef(RE::StaticFunctionTag*, RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled, int projectileType) {
    RE::TESObjectREFR* returnRef = nullptr;
    if (!gfuncs::IsFormValid(ref)) {
        logger::warn("ref isn't valid or doesn't exist");
        return returnRef;
    }

    auto it = recentHitProjectiles.find(ref);
    if (it != recentHitProjectiles.end()) {
        if (it->second.size() == 0) {
            return nullptr;
        }
        if (projectileType >= 1 && projectileType <= 7) {
            for (int i = it->second.size() - 1; i >= 0 && !returnRef; --i) {
                if (GetProjectileRefType(nullptr, it->second[i].projectile) == projectileType) {
                    if ((it->second[i].projectile->Is3DLoaded() || !only3dLoaded) && (!it->second[i].projectile->IsDisabled() || !onlyEnabled)) {
                        returnRef = it->second[i].projectile;
                    }
                }
            }
        }
        else {
            for (int i = it->second.size() - 1; i >= 0 && !returnRef; --i) {
                if (gfuncs::IsFormValid(it->second[i].projectile)) {
                    if ((it->second[i].projectile->Is3DLoaded() || !only3dLoaded) && (!it->second[i].projectile->IsDisabled() || !onlyEnabled)) {
                        returnRef = it->second[i].projectile;
                    }
                }
            }
        }
    }
    return returnRef;
}

std::vector<RE::TESObjectREFR*> GetRecentProjectileShotRefs(RE::StaticFunctionTag*, RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled, int projectileType) {
    std::vector<RE::TESObjectREFR*> refs;
    if (!gfuncs::IsFormValid(ref)) {
        logger::warn("ref doesn't exist");
        return refs;
    }

    auto it = recentShotProjectiles.find(ref);
    if (it != recentShotProjectiles.end()) {
        if (it->second.size() == 0) {
            return refs;
        }
        if (projectileType >= 1 && projectileType <= 7) {
            for (auto& data : it->second) {
                if (GetProjectileRefType(nullptr, data.projectile) == projectileType) {
                    if ((data.projectile->Is3DLoaded() || !only3dLoaded) && (!data.projectile->IsDisabled() || !onlyEnabled)) {
                        refs.push_back(data.projectile);
                    }
                }
            }
        }
        else {
            for (auto& data : it->second) {
                if (gfuncs::IsFormValid(data.projectile)) {
                    if ((data.projectile->Is3DLoaded() || !only3dLoaded) && (!data.projectile->IsDisabled() || !onlyEnabled)) {
                        refs.push_back(data.projectile);
                    }
                }
            }
        }
    }
    return refs;
}

RE::TESObjectREFR* GetLastProjectileShotRef(RE::StaticFunctionTag*, RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled, int projectileType) {
    RE::TESObjectREFR* returnRef = nullptr;
    if (!gfuncs::IsFormValid(ref)) {
        logger::warn("ref isn't valid or doesn't exist");
        return returnRef;
    }

    auto it = recentShotProjectiles.find(ref);
    if (it != recentShotProjectiles.end()) {
        if (it->second.size() == 0) {
            return nullptr;
        }
        if (projectileType >= 1 && projectileType <= 7) {
            for (int i = it->second.size() - 1; i >= 0 && !returnRef; --i) {
                if (GetProjectileRefType(nullptr, it->second[i].projectile) == projectileType) {
                    if ((it->second[i].projectile->Is3DLoaded() || !only3dLoaded) && (!it->second[i].projectile->IsDisabled() || !onlyEnabled)) {
                        returnRef = it->second[i].projectile;
                    }
                }
            }
        }
        else {
            for (int i = it->second.size() - 1; i >= 0 && !returnRef; --i) {
                if (gfuncs::IsFormValid(it->second[i].projectile)) {
                    if ((it->second[i].projectile->Is3DLoaded() || !only3dLoaded) && (!it->second[i].projectile->IsDisabled() || !onlyEnabled)) {
                        returnRef = it->second[i].projectile;
                    }
                }
            }
        }
    }
    return returnRef;
}

std::vector<RE::TESObjectREFR*> GetAttachedProjectileRefs(RE::StaticFunctionTag*, RE::TESObjectREFR* ref) {
    logger::trace("called");

    std::vector<RE::TESObjectREFR*> attachedProjectiles;
    if (!gfuncs::IsFormValid(ref)) {
        logger::warn("ref isn't valid or doesn't exist");
        return attachedProjectiles;
    }

    const auto& [allForms, lock] = RE::TESForm::GetAllForms();
    for (auto& [id, form] : *allForms) {
        auto* akRef = form->AsReference();
        if (gfuncs::IsFormValid(akRef)) {
            RE::Projectile* projectileRef = akRef->AsProjectile();
            if (DidProjectileHitRef(projectileRef, ref)) {
                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                    int impactResult = GetProjectileImpactResult(nullptr, projectileRef);
                    if (impactResult == 3 || impactResult == 4) { //impale or stick
                        float distance = akRef->GetPosition().GetDistance(ref->GetPosition());
                        if (distance < 3000.0) {
                            attachedProjectiles.push_back(akRef);
                        }
                    }
                }
            }
        }
    }
    return attachedProjectiles;
}

std::vector<RE::BGSProjectile*> GetAttachedProjectiles(RE::StaticFunctionTag*, RE::TESObjectREFR* ref) {
    std::vector<RE::BGSProjectile*> attachedProjectiles;

    if (!gfuncs::IsFormValid(ref)) {
        logger::warn("ref isn't valid or doesn't exist");
        return attachedProjectiles;
    }

    auto* data = ref->extraList.GetByType<RE::ExtraAttachedArrows3D>();
    if (data) {
        if (!data->data.empty()) {
            for (auto& dataItem : data->data) {
                if (dataItem.source) {
                    attachedProjectiles.push_back(dataItem.source);
                }
            }
        }
    }
    else {
        logger::debug("couldn't get ExtraAttachedArrows3D for ref[{}]", gfuncs::GetFormName(ref));
    }
    return attachedProjectiles;
}

//Get all hit projectiles of type =======================================================================================================
//All Projectiles that hit ref
std::vector<RE::TESObjectREFR*> GetAllHitProjectileRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;

    if (!gfuncs::IsFormValid(ref)) {
        logger::warn("ref isn't valid or doesn't exist");
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::Projectile* projectileRef = akRef->AsProjectile();
                if (DidProjectileHitRef(projectileRef, ref)) {
                    if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                        projectileRefs.push_back(akRef);
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::Projectile* projectileRef = akRef->AsProjectile();
                if (DidProjectileHitRef(projectileRef, ref)) {
                    if (!akRef->IsDisabled()) {
                        projectileRefs.push_back(akRef);
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::Projectile* projectileRef = akRef->AsProjectile();
                if (DidProjectileHitRef(projectileRef, ref)) {
                    if (akRef->Is3DLoaded()) {
                        projectileRefs.push_back(akRef);
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::Projectile* projectileRef = akRef->AsProjectile();
                if (DidProjectileHitRef(projectileRef, ref)) {
                    projectileRefs.push_back(akRef);
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 7 arrow
std::vector<RE::TESObjectREFR*> GetAllHitProjectileArrowRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;

    if (!gfuncs::IsFormValid(ref)) {
        logger::warn("ref doesn't exist or isn't valid");
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsArrow()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsArrow()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsArrow()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsArrow()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 6 barrier
std::vector<RE::TESObjectREFR*> GetAllHitProjectileBarrierRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("ref doesn't exist");
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBarrier()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBarrier()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBarrier()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBarrier()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 5 cone
std::vector<RE::TESObjectREFR*> GetAllHitProjectileConeRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("ref doesn't exist");
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsCone()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsCone()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsCone()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsCone()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 4 Flamethrower
std::vector<RE::TESObjectREFR*> GetAllHitProjectileFlamethrowerRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("ref doesn't exist");
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsFlamethrower()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsFlamethrower()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsFlamethrower()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsFlamethrower()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 3 Beam
std::vector<RE::TESObjectREFR*> GetAllHitProjectileBeamRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("ref doesn't exist");
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBeam()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBeam()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBeam()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBeam()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 2 Grenade
std::vector<RE::TESObjectREFR*> GetAllHitProjectileGrenadeRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("ref doesn't exist");
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsGrenade()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsGrenade()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsGrenade()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsGrenade()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 1 Missile
std::vector<RE::TESObjectREFR*> GetAllHitProjectileMissileRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("ref doesn't exist");
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsMissile()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsMissile()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsMissile()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsMissile()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidProjectileHitRef(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

std::vector<RE::TESObjectREFR*> GetAllHitProjectileRefsOfType(RE::StaticFunctionTag*, RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled, int projectileType) {
    switch (projectileType) {
    case 1:
        return GetAllHitProjectileMissileRefs(ref, only3dLoaded, onlyEnabled);
    case 2:
        return GetAllHitProjectileGrenadeRefs(ref, only3dLoaded, onlyEnabled);
    case 3:
        return GetAllHitProjectileBeamRefs(ref, only3dLoaded, onlyEnabled);
    case 4:
        return GetAllHitProjectileFlamethrowerRefs(ref, only3dLoaded, onlyEnabled);
    case 5:
        return GetAllHitProjectileConeRefs(ref, only3dLoaded, onlyEnabled);
    case 6:
        return GetAllHitProjectileBarrierRefs(ref, only3dLoaded, onlyEnabled);
    case 7:
        return GetAllHitProjectileArrowRefs(ref, only3dLoaded, onlyEnabled);
    default:
        return GetAllHitProjectileRefs(ref, only3dLoaded, onlyEnabled);
    }
}

//=======================================================================================================================================================

//Get all shot arrows of type ===========================================================================================================================
//All Projectiles that the ref shot
std::vector<RE::TESObjectREFR*> GetAllShotProjectileRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("ref doesn't exist");
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::Projectile* projectileRef = akRef->AsProjectile();
                if (DidRefShootProjectile(projectileRef, ref)) {
                    if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                        projectileRefs.push_back(akRef);
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::Projectile* projectileRef = akRef->AsProjectile();
                if (DidRefShootProjectile(projectileRef, ref)) {
                    if (!akRef->IsDisabled()) {
                        projectileRefs.push_back(akRef);
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::Projectile* projectileRef = akRef->AsProjectile();
                if (DidRefShootProjectile(projectileRef, ref)) {
                    if (akRef->Is3DLoaded()) {
                        projectileRefs.push_back(akRef);
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::Projectile* projectileRef = akRef->AsProjectile();
                if (DidRefShootProjectile(projectileRef, ref)) {
                    projectileRefs.push_back(akRef);
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 7 arrow
std::vector<RE::TESObjectREFR*> GetAllShotProjectileArrowRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("ref doesn't exist");
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsArrow()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsArrow()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsArrow()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsArrow()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 6 barrier
std::vector<RE::TESObjectREFR*> GetAllShotProjectileBarrierRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("ref doesn't exist");
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBarrier()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBarrier()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBarrier()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBarrier()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 5 cone
std::vector<RE::TESObjectREFR*> GetAllShotProjectileConeRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("ref doesn't exist");
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsCone()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsCone()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsCone()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsCone()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 4 Flamethrower
std::vector<RE::TESObjectREFR*> GetAllShotProjectileFlamethrowerRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("ref doesn't exist");
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsFlamethrower()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsFlamethrower()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsFlamethrower()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsFlamethrower()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 3 Beam
std::vector<RE::TESObjectREFR*> GetAllShotProjectileBeamRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("ref doesn't exist");
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBeam()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBeam()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBeam()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsBeam()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 2 Grenade
std::vector<RE::TESObjectREFR*> GetAllShotProjectileGrenadeRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("ref doesn't exist");
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsGrenade()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsGrenade()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsGrenade()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsGrenade()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

//projectile type 1 Missile
std::vector<RE::TESObjectREFR*> GetAllShotProjectileMissileRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled) {
    std::vector<RE::TESObjectREFR*> projectileRefs;
    if (!ref) {
        logger::warn("ref doesn't exist");
        return projectileRefs;
    }

    if (only3dLoaded && onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsMissile()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled() && akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (onlyEnabled) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsMissile()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (!akRef->IsDisabled()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if (only3dLoaded) {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsMissile()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                if (akRef->Is3DLoaded()) {
                                    projectileRefs.push_back(akRef);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        const auto& [allForms, lock] = RE::TESForm::GetAllForms();
        for (auto& [id, form] : *allForms) {
            auto* akRef = form->AsReference();
            if (gfuncs::IsFormValid(akRef)) {
                RE::TESForm* baseForm = akRef->GetBaseObject();
                if (gfuncs::IsFormValid(baseForm)) {
                    RE::BGSProjectile* projectile = baseForm->As<RE::BGSProjectile>();
                    if (gfuncs::IsFormValid(projectile)) {
                        if (projectile->IsMissile()) {
                            RE::Projectile* projectileRef = akRef->AsProjectile();
                            if (DidRefShootProjectile(projectileRef, ref)) {
                                projectileRefs.push_back(akRef);
                            }
                        }
                    }
                }
            }
        }
    }

    return projectileRefs;
}

std::vector<RE::TESObjectREFR*> GetAllShotProjectileRefsOfType(RE::StaticFunctionTag*, RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled, int projectileType) {
    switch (projectileType) {
    case 1:
        return GetAllShotProjectileMissileRefs(ref, only3dLoaded, onlyEnabled);
    case 2:
        return GetAllShotProjectileGrenadeRefs(ref, only3dLoaded, onlyEnabled);
    case 3:
        return GetAllShotProjectileBeamRefs(ref, only3dLoaded, onlyEnabled);
    case 4:
        return GetAllShotProjectileFlamethrowerRefs(ref, only3dLoaded, onlyEnabled);
    case 5:
        return GetAllShotProjectileConeRefs(ref, only3dLoaded, onlyEnabled);
    case 6:
        return GetAllShotProjectileBarrierRefs(ref, only3dLoaded, onlyEnabled);
    case 7:
        return GetAllShotProjectileArrowRefs(ref, only3dLoaded, onlyEnabled);
    default:
        return GetAllShotProjectileRefs(ref, only3dLoaded, onlyEnabled);
    }
}
