#pragma once

struct TrackedProjectileData {
	RE::Projectile* projectile;
	RE::TESObjectREFR* shooter;
	RE::TESObjectREFR* target;
	RE::TESAmmo* ammo;
	float gameTimeStamp; //game time when the projectile last had an impact event
	float lastImpactEventGameTimeStamp; //last time a projectile impact event was sent for this data
	RE::BGSProjectile* projectileBase;
	int impactResult;
	int collidedLayer;
	float distanceTraveled;
	std::string hitPartNodeName;
	RE::TESObjectREFR* projectileMarker;
	//RE::TESObjectREFR* targetMarker;

	//std::chrono::system_clock::time_point timeStamp;
	//uint32_t runTimeStamp;
}; 

extern std::map<RE::TESObjectREFR*, std::vector<TrackedProjectileData>> recentHitProjectiles;
extern std::map<RE::TESObjectREFR*, std::vector<TrackedProjectileData>> recentShotProjectiles;

bool DidShooterHitRefWithProjectile(RE::TESObjectREFR* shooter, RE::TESObjectREFR* ref, TrackedProjectileData& data);

bool DidProjectileHitRef(RE::Projectile * akProjectile, RE::TESObjectREFR * ref);

bool DidRefShootProjectile(RE::Projectile * akProjectile, RE::TESObjectREFR * ref);

bool DidProjectileHitRefWithAmmoFromShooter(RE::TESObjectREFR * shooter, RE::TESObjectREFR * ref, RE::TESAmmo * akAmmo, TrackedProjectileData & data);

bool DidProjectileHitRefWithAmmoFromShooter(RE::TESObjectREFR * shooter, RE::TESObjectREFR * ref, RE::Projectile * akProjectile, RE::TESAmmo * akAmmo);

int GetProjectileType(RE::BGSProjectile * projectile);

RE::BGSTextureSet* GetProjectileBaseDecal(RE::StaticFunctionTag*, RE::BGSProjectile * projectile);

bool SetProjectileBaseDecal(RE::StaticFunctionTag*, RE::BGSProjectile * projectile, RE::BGSTextureSet * decalTextureSet);

RE::BGSExplosion* GetProjectileBaseExplosion(RE::StaticFunctionTag*, RE::BGSProjectile * projectile);

bool SetProjectileBaseExplosion(RE::StaticFunctionTag*, RE::BGSProjectile * projectile, RE::BGSExplosion * akExplosion);

float GetProjectileBaseCollisionRadius(RE::StaticFunctionTag*, RE::BGSProjectile * projectile);

bool SetProjectileBaseCollisionRadius(RE::StaticFunctionTag*, RE::BGSProjectile * projectile, float radius);

float GetProjectileBaseCollisionConeSpread(RE::StaticFunctionTag*, RE::BGSProjectile * projectile);

bool SetProjectileBaseCollisionConeSpread(RE::StaticFunctionTag*, RE::BGSProjectile * projectile, float coneSpread);

int GetProjectileRefType(RE::StaticFunctionTag*, RE::TESObjectREFR * projectileRef);

RE::TESObjectREFR* GetProjectileHitRef(RE::Projectile::ImpactData * impactData);

std::vector<RE::TESObjectREFR*> GetProjectileHitRefs(RE::StaticFunctionTag*, RE::TESObjectREFR * projectileRef);

RE::TESObjectREFR* GetLastProjectileHitRefFromRuntimeData(RE::Projectile::PROJECTILE_RUNTIME_DATA & runtimeData);

RE::TESObjectREFR* GetProjectileShooterFromRuntimeData(RE::Projectile::PROJECTILE_RUNTIME_DATA & runtimeData);

RE::TESObjectREFR* GetProjectileShooter(RE::StaticFunctionTag*, RE::TESObjectREFR * projectileRef);

RE::BGSExplosion* GetProjectileExplosion(RE::StaticFunctionTag*, RE::TESObjectREFR * projectileRef);

RE::TESAmmo* GetProjectileAmmoSource(RE::StaticFunctionTag*, RE::TESObjectREFR * projectileRef);

RE::AlchemyItem* GetProjectilePoison(RE::StaticFunctionTag*, RE::TESObjectREFR * projectileRef);

RE::EnchantmentItem* GetProjectileEnchantment(RE::StaticFunctionTag*, RE::TESObjectREFR * projectileRef);

RE::TESForm* GetProjectileMagicSource(RE::StaticFunctionTag*, RE::TESObjectREFR * projectileRef);

RE::TESObjectWEAP* GetProjectileWeaponSource(RE::StaticFunctionTag*, RE::TESObjectREFR * projectileRef);

float GetProjectileWeaponDamage(RE::StaticFunctionTag*, RE::TESObjectREFR * projectileRef);

float GetProjectilePower(RE::StaticFunctionTag*, RE::TESObjectREFR * projectileRef);

float GetProjectileDistanceTraveled(RE::StaticFunctionTag*, RE::TESObjectREFR * projectileRef);

int GetProjectileImpactResult(RE::StaticFunctionTag*, RE::TESObjectREFR * projectileRef);

std::vector<int> GetProjectileCollidedLayers(RE::StaticFunctionTag*, RE::TESObjectREFR * projectileRef);

std::string GetCollisionLayerName(RE::StaticFunctionTag*, int layer);

std::vector<std::string> GetProjectileCollidedLayerNames(RE::StaticFunctionTag*, RE::TESObjectREFR * projectileRef);

std::string GetProjectileNodeHitName(RE::Projectile::ImpactData * impactData);

std::vector<std::string> GetProjectileNodeHitNames(RE::StaticFunctionTag*, RE::TESObjectREFR * projectileRef);

std::vector<RE::TESObjectREFR*> GetRecentProjectileHitRefs(RE::StaticFunctionTag*, RE::TESObjectREFR * ref, bool only3dLoaded, bool onlyEnabled, int projectileType);

RE::TESObjectREFR* GetLastProjectileHitRef(RE::StaticFunctionTag*, RE::TESObjectREFR * ref, bool only3dLoaded, bool onlyEnabled, int projectileType);

std::vector<RE::TESObjectREFR*> GetRecentProjectileShotRefs(RE::StaticFunctionTag*, RE::TESObjectREFR * ref, bool only3dLoaded, bool onlyEnabled, int projectileType);

RE::TESObjectREFR* GetLastProjectileShotRef(RE::StaticFunctionTag*, RE::TESObjectREFR * ref, bool only3dLoaded, bool onlyEnabled, int projectileType);

std::vector<RE::TESObjectREFR*> GetAttachedProjectileRefs(RE::StaticFunctionTag*, RE::TESObjectREFR * ref);

std::vector<RE::BGSProjectile*> GetAttachedProjectiles(RE::StaticFunctionTag*, RE::TESObjectREFR * ref);

std::vector<RE::TESObjectREFR*> GetAllHitProjectileRefsOfType(RE::StaticFunctionTag*, RE::TESObjectREFR * ref, bool only3dLoaded, bool onlyEnabled, int projectileType);

std::vector<RE::TESObjectREFR*> GetAllShotProjectileRefsOfType(RE::StaticFunctionTag*, RE::TESObjectREFR * ref, bool only3dLoaded, bool onlyEnabled, int projectileType);

std::vector<RE::TESObjectREFR*> GetAllShotProjectileRefs(RE::TESObjectREFR* ref, bool only3dLoaded, bool onlyEnabled);
