#include "Package.h"
#include "GeneralFunctions.h"
#include "magic_enum/magic_enum.hpp"
#include "magic_enum/magic_enum_all.hpp"
#include "RE/B/BGSIdleCollection.h"
#include "RE/P/PackageLocation.h"
#include "RE/T/TESCombatStyle.h"
#include "RE/T/TESIdleForm.h"
#include "RE/T/TESObjectCELL.h"
#include "RE/T/TESPackage.h"
#include "RE/T/TESQuest.h"
#include "RE/T/TypeTraits.h"

namespace Package {

	//helpers ======================================================================================================================================

	//Papyrus index -> PACKAGE_DATA::GeneralFlag bit. Index 0 is kNone.
	constexpr std::array<RE::PACKAGE_DATA::GeneralFlag, 17> kGeneralFlags{
		RE::PACKAGE_DATA::GeneralFlag::kNone,
		RE::PACKAGE_DATA::GeneralFlag::kOffersServices,               // 1
		RE::PACKAGE_DATA::GeneralFlag::kMustComplete,                 // 2
		RE::PACKAGE_DATA::GeneralFlag::kMaintainSpeedAtGoal,          // 3
		RE::PACKAGE_DATA::GeneralFlag::kUnlocksDoorsAtPackageStart,   // 4
		RE::PACKAGE_DATA::GeneralFlag::kUnlocksDoorsAtPackageEnd,     // 5
		RE::PACKAGE_DATA::GeneralFlag::kContinueIfPCNear,             // 6
		RE::PACKAGE_DATA::GeneralFlag::kOncePerDay,                   // 7
		RE::PACKAGE_DATA::GeneralFlag::kCreated,                      // 8
		RE::PACKAGE_DATA::GeneralFlag::kPreferredSpeed,               // 9
		RE::PACKAGE_DATA::GeneralFlag::kAlwaysSneak,                  // 10
		RE::PACKAGE_DATA::GeneralFlag::kAllowSwimming,                // 11
		RE::PACKAGE_DATA::GeneralFlag::kIgnoreCombat,                 // 12
		RE::PACKAGE_DATA::GeneralFlag::kWeaponsUnequipped,            // 13
		RE::PACKAGE_DATA::GeneralFlag::kWeaponDrawn,                  // 14
		RE::PACKAGE_DATA::GeneralFlag::kNoCombatAlert,                // 15
		RE::PACKAGE_DATA::GeneralFlag::kWearSleepOutfit               // 16
	};

	//Papyrus index -> PACKAGE_DATA::InterruptFlag bit. Index 0 is kNone. Note bit 8 is unused, 9 is 1 << 9.
	constexpr std::array<RE::PACKAGE_DATA::InterruptFlag, 10> kInterruptFlags{
		RE::PACKAGE_DATA::InterruptFlag::kNone,
		RE::PACKAGE_DATA::InterruptFlag::kHellosToPlayer,             // 1
		RE::PACKAGE_DATA::InterruptFlag::kRandomConversations,        // 2
		RE::PACKAGE_DATA::InterruptFlag::kObserveCombatBehaviour,     // 3
		RE::PACKAGE_DATA::InterruptFlag::kGreetCorpseBehaviour,       // 4
		RE::PACKAGE_DATA::InterruptFlag::kReactionToPlayerActions,    // 5
		RE::PACKAGE_DATA::InterruptFlag::kFriendlyFireComments,       // 6
		RE::PACKAGE_DATA::InterruptFlag::kAggroRadiusBehavior,        // 7
		RE::PACKAGE_DATA::InterruptFlag::kAllowIdleChatter,           // 8
		RE::PACKAGE_DATA::InterruptFlag::kWorldInteractions           // 9
	};

	//Papyrus index -> BGSIdleCollection::IdleFlags bit. Index 0 is kNone.
	constexpr std::array<RE::BGSIdleCollection::IdleFlags, 6> kIdleFlags{
		RE::BGSIdleCollection::IdleFlags::kNone,
		RE::BGSIdleCollection::IdleFlags::kPickSequence,        // 1
		RE::BGSIdleCollection::IdleFlags::kOldPickConditions,   // 2
		RE::BGSIdleCollection::IdleFlags::kDoOnce,              // 3
		RE::BGSIdleCollection::IdleFlags::kLooseOnly,           // 4
		RE::BGSIdleCollection::IdleFlags::kNoSandbox            // 5
	};

	//PackageTarget::targType values (PTDA in the CK / xEdit)
	enum class TargetType : std::int8_t {
		kSpecificReference = 0,  //target.handle
		kObjectID = 1,           //target.object
		kObjectType = 2,         //target.objType
		kLinkedReference = 3,
		kRefAlias = 4,           //target.aliasID
		kInterruptData = 5,
		kSelf = 6,
		kKeyword = 7,
	};

	//Template-based packages (procedure type kPackage) keep their parameters here instead of packLoc / packTarg.
	RE::TESCustomPackageData* GetCustomData(RE::TESPackage* package) {
		return (package && package->data) ? skyrim_cast<RE::TESCustomPackageData*>(package->data) : nullptr;
	}

	//Engine type names: "Location", "TargetSelector", "SingleRef", "Bool", "Int", "Float", "ObjectList", "Topic", ...
	const char* GetDataTypeName(RE::IPackageData* a_item) {
		const char* name = a_item ? a_item->GetTypeName().c_str() : nullptr;
		return name ? name : "";
	}

	RE::IPackageData* FindPackageData(RE::TESPackage* package, const char* a_typeName) {
		auto* custom = GetCustomData(package);
		if (!custom || !custom->data.data) {
			return nullptr;
		}
		for (std::uint16_t i = 0; i < custom->data.dataSize; i++) {
			auto* item = custom->data.data[i];
			if (item && _stricmp(GetDataTypeName(item), a_typeName) == 0) {
				return item;
			}
		}
		return nullptr;
	}

	//old-style packLoc, else the first "Location" entry of a template package
	RE::PackageLocation* GetPackageLocationPtr(RE::TESPackage* package) {
		if (package->packLoc) {
			return package->packLoc;
		}
		if (auto* item = FindPackageData(package, "Location")) {
			if (auto* loc = skyrim_cast<RE::BGSPackageDataLocation*>(item)) {
				return loc->pointer;
			}
		}
		return nullptr;
	}

	//old-style packTarg, else the first "TargetSelector" entry of a template package
	RE::PackageTarget* GetPackageTargetPtr(RE::TESPackage* package) {
		if (package->packTarg) {
			return package->packTarg;
		}
		if (auto* item = FindPackageData(package, "TargetSelector")) {
			//BGSPackageDataTargetSelector isn't in CommonLib. Layout from its template base:
			//IPackageData vtable (0x00), data union (0x08), PackageTarget* pointer (0x10)
			return *reinterpret_cast<RE::PackageTarget**>(reinterpret_cast<std::uintptr_t>(item) + 0x10);
		}
		return nullptr;
	}
	
	template <class T, std::size_t N>
	bool IndexInRange(int index, const std::array<T, N>& table, const char* what) {
		if (index < 1 || index >= static_cast<int>(N)) {
			logger::error("{} [{}] is out of range, must be 1 to {}", what, index, N - 1);
			return false;
		}
		return true;
	}

	RE::TESObjectREFR* RefFromHandle(const RE::ObjectRefHandle& handle) {
		auto refPtr = handle.get();
		return refPtr ? refPtr.get() : nullptr;
	}

	//PACKAGE_DATA ===================================================================================================================================

	// kNone = 0
	// kOffersServices = 1
	// kMustComplete = 2
	// kMaintainSpeedAtGoal = 3
	// kUnlocksDoorsAtPackageStart = 4
	// kUnlocksDoorsAtPackageEnd = 5
	// kContinueIfPCNear = 6
	// kOncePerDay = 7
	// kCreated = 8
	// kPreferredSpeed = 9
	// kAlwaysSneak = 10
	// kAllowSwimming = 11
	// kIgnoreCombat = 12
	// kWeaponsUnequipped = 13
	// kWeaponDrawn = 14
	// kNoCombatAlert = 15
	// kWearSleepOutfit = 16
	bool GetPackageFlagSet(RE::StaticFunctionTag*, RE::TESPackage* akPackage, int flag) {
		if (!gfuncs::IsFormValid(akPackage) || !IndexInRange(flag, kGeneralFlags, "package flag")) {
			return false;
		}
		return akPackage->packData.packFlags.any(kGeneralFlags[flag]);
	}

	void SetPackageFlag(RE::StaticFunctionTag*, RE::TESPackage* akPackage, int flag, bool set) {
		if (!gfuncs::IsFormValid(akPackage) || !IndexInRange(flag, kGeneralFlags, "flag")) {
			return;
		}
		if (set) {
			akPackage->packData.packFlags.set(kGeneralFlags[flag]);
		}
		else {
			akPackage->packData.packFlags.reset(kGeneralFlags[flag]);
		}
	}

	//0 = walk, 1 = jog, 2 = run, 3 = fast walk. -1 if the package doesn't use a preferred speed (flag 9 not set).
	int GetPackagePreferredSpeed(RE::StaticFunctionTag*, RE::TESPackage* akPackage) {
		if (!gfuncs::IsFormValid(akPackage)) {
			return -1;
		}
		if (!akPackage->packData.packFlags.any(RE::PACKAGE_DATA::GeneralFlag::kPreferredSpeed)) {
			return -1;
		}
		return static_cast<int>(akPackage->packData.maxSpeed.underlying());
	}

	//speed 0 to 3 sets the speed and turns on the Preferred Speed flag. speed -1 turns the flag off.
	void SetPackagePreferredSpeed(RE::StaticFunctionTag*, RE::TESPackage* akPackage, int speed) {
		if (!gfuncs::IsFormValid(akPackage)) {
			return;
		}
		if (speed == -1) {
			akPackage->packData.packFlags.reset(RE::PACKAGE_DATA::GeneralFlag::kPreferredSpeed);
			return;
		}
		if (speed < 0 || speed > 3) {
			logger::error("speed [{}] is out of range, must be -1 to 3", speed);
			return;
		}
		akPackage->packData.maxSpeed = static_cast<RE::PACKAGE_DATA::PreferredSpeed>(speed);
		akPackage->packData.packFlags.set(RE::PACKAGE_DATA::GeneralFlag::kPreferredSpeed);
	}

	// kNone = 0
	// kHellosToPlayer = 1
	// kRandomConversations = 2
	// kObserveCombatBehaviour = 3
	// kGreetCorpseBehaviour = 4
	// kReactionToPlayerActions = 5
	// kFriendlyFireComments = 6
	// kAggroRadiusBehavior = 7
	// kAllowIdleChatter = 8
	// kWorldInteractions = 9
	bool GetPackageInterruptFlag(RE::StaticFunctionTag*, RE::TESPackage* akPackage, int flag) {
		if (!gfuncs::IsFormValid(akPackage) || !IndexInRange(flag, kInterruptFlags, "interrupt flag")) {
			return false;
		}
		return akPackage->packData.foBehaviorFlags.any(kInterruptFlags[flag]);
	}

	void SetPackageInterruptFlag(RE::StaticFunctionTag*, RE::TESPackage* akPackage, int flag, bool set) {
		if (!gfuncs::IsFormValid(akPackage) || !IndexInRange(flag, kInterruptFlags, "interrupt flag")) {
			return;
		}
		if (set) {
			akPackage->packData.foBehaviorFlags.set(kInterruptFlags[flag]);
		}
		else {
			akPackage->packData.foBehaviorFlags.reset(kInterruptFlags[flag]);
		}
	}

	//0 = none, 1 = spectator, 2 = observe dead, 3 = guard warn, 4 = combat
	int GetPackageInterruptOverrideType(RE::StaticFunctionTag*, RE::TESPackage* akPackage) {
		if (!gfuncs::IsFormValid(akPackage)) {
			return -1;
		}
		return static_cast<int>(akPackage->packData.interruptOverrideType.underlying());
	}

	void SetPackageInterruptOverrideType(RE::StaticFunctionTag*, RE::TESPackage* akPackage, int type) {
		if (!gfuncs::IsFormValid(akPackage)) {
			return;
		}
		if (type < 0 || type > 4) {
			logger::error("interrupt override type [{}] is out of range, must be 0 to 4", type);
			return;
		}
		akPackage->packData.interruptOverrideType = static_cast<RE::PACK_INTERRUPT_TARGET>(type);
	}

	//PackageLocation ================================================================================================================================

	// kNone = -1
	// kNearReference = 0
	// kInCell = 1
	// kNearPackageStartLocation = 2
	// kNearEditorLocation = 3
	// kObjectID = 4
	// kObjectType = 5
	// kNearLinkedReference = 6
	// kAtPackagelocation = 7
	// kAlias_Reference = 8
	// kAlias_Location = 9
	// kNearSelf = 12
	int GetPackageLocationType(RE::StaticFunctionTag*, RE::TESPackage* akPackage) {
		if (!gfuncs::IsFormValid(akPackage)) {
			return -1;
		}
		auto* loc = GetPackageLocationPtr(akPackage);
		if (!loc) {
			return -1;
		}
		return static_cast<int>(static_cast<std::int8_t>(loc->locType.underlying()));
	}

	//Only for location types that don't need any data: 2 (near package start), 3 (near editor location), 12 (near self).
	//For the others use SetPackageLocationRef / SetPackageLocationForm, which set the type and its data together,
	//because the location data means something different for each type.
	bool SetPackageLocationType(RE::StaticFunctionTag*, RE::TESPackage* akPackage, int type) {
		if (!gfuncs::IsFormValid(akPackage)) {
			return false;
		}
		auto* loc = GetPackageLocationPtr(akPackage);
		if (!loc) {
			return false;
		}
		if (type != 2 && type != 3 && type != 12) {
			logger::error("location type [{}] needs data; use SetPackageLocationRef or SetPackageLocationForm", type);
			return false;
		}
		loc->data.object = nullptr;
		loc->locType = static_cast<RE::PackageLocation::Type>(type);
		return true;
	}

	//returns the cell (type 1 in cell) or object (type 4 object ID), else none
	RE::TESForm* GetPackageLocationForm(RE::StaticFunctionTag*, RE::TESPackage* akPackage) {
		if (!gfuncs::IsFormValid(akPackage)) {
			return nullptr;
		}
		auto* loc = GetPackageLocationPtr(akPackage);
		if (!loc) {
			return nullptr;
		}
		if (loc->locType == RE::PackageLocation::Type::kInCell || loc->locType == RE::PackageLocation::Type::kObjectID) {
			return loc->data.object;
		}
		return nullptr;
	}

	//a cell sets type 1 (in cell), any other form sets type 4 (object ID)
	bool SetPackageLocationForm(RE::StaticFunctionTag*, RE::TESPackage* akPackage, RE::TESForm* akForm) {
		if (!gfuncs::IsFormValid(akPackage) || !gfuncs::IsFormValid(akForm)) {
			return false;
		}
		auto* loc = GetPackageLocationPtr(akPackage);
		if (!loc) {
			return false;
		}
		loc->data.object = akForm;
		loc->locType = akForm->Is(RE::FormType::Cell) ? RE::PackageLocation::Type::kInCell : RE::PackageLocation::Type::kObjectID;
		return true;
	}

	//returns the reference for type 0 (near reference), else none
	RE::TESObjectREFR* GetPackageLocationRef(RE::StaticFunctionTag*, RE::TESPackage* akPackage) {
		if (!gfuncs::IsFormValid(akPackage)) {
			return nullptr;
		}
		auto* loc = GetPackageLocationPtr(akPackage);
		if (!loc) {
			return nullptr;
		}
		if (loc->locType != RE::PackageLocation::Type::kNearReference) {
			return nullptr;
		}
		return RefFromHandle(loc->data.refHandle);
	}

	//sets type 0 (near reference)
	bool SetPackageLocationRef(RE::StaticFunctionTag*, RE::TESPackage* akPackage, RE::TESObjectREFR* akRef) {
		if (!gfuncs::IsFormValid(akPackage) || !gfuncs::IsFormValid(akRef)) {
			return false;
		}
		
		auto* loc = GetPackageLocationPtr(akPackage);
		if (!loc) {
			return false;
		}
		
		loc->data.object = nullptr;  //clear the full 8 bytes before writing the 4 byte handle
		loc->data.refHandle = akRef->CreateRefHandle();
		loc->locType = RE::PackageLocation::Type::kNearReference;
		return true;
	}

	int GetPackageLocationRadius(RE::StaticFunctionTag*, RE::TESPackage* akPackage) {
		if (!gfuncs::IsFormValid(akPackage)) {
			return -1; 
		}
		auto* loc = GetPackageLocationPtr(akPackage);
		if (!loc) {
			return -1;
		}
		
		return static_cast<int>(loc->rad);
	}

	void SetPackageLocationRadius(RE::StaticFunctionTag*, RE::TESPackage* akPackage, int radius) {
		if (!gfuncs::IsFormValid(akPackage) || radius < 0) {
			return;
		}
		auto* loc = GetPackageLocationPtr(akPackage);
		if (!loc) {
			return;
		}
		
		loc->rad = static_cast<std::uint32_t>(radius);
	}

	//PackageTarget ==================================================================================================================================

	
	// 0 = specific reference, 1 = object ID, 2 = object type, 3 = linked reference,
	// 4 = ref alias, 5 = interrupt data, 6 = self, 7 = keyword. -1 if the package has no target.
	int GetPackageTargetType(RE::StaticFunctionTag*, RE::TESPackage* akPackage) {
		auto* targ = gfuncs::IsFormValid(akPackage) ? GetPackageTargetPtr(akPackage) : nullptr;
		return targ ? static_cast<int>(targ->targType) : -1;
	}

	//returns the reference (type 0) or object (types 1 and 7), else none
	RE::TESForm* GetPackageTarget(RE::StaticFunctionTag*, RE::TESPackage* akPackage) {
		auto* targ = gfuncs::IsFormValid(akPackage) ? GetPackageTargetPtr(akPackage) : nullptr;
		if (!targ) {
			return nullptr;
		}
		switch (static_cast<TargetType>(targ->targType)) {
			case TargetType::kSpecificReference:
				return RefFromHandle(targ->target.handle);
			case TargetType::kObjectID:
			case TargetType::kKeyword:
				return targ->target.object;
			default:
				return nullptr;
		}
	}

	//a reference sets type 0 (specific reference), a keyword type 7, any other form type 1 (object ID)
	bool SetPackageTarget(RE::StaticFunctionTag*, RE::TESPackage* akPackage, RE::TESForm* akForm) {
		if (!gfuncs::IsFormValid(akPackage) || !gfuncs::IsFormValid(akForm)) {
			return false;
		}
		auto* targ = GetPackageTargetPtr(akPackage);
		if (!targ) {
			return false;
		}
		targ->target.object = nullptr;  //clear the full 8 bytes

		if (auto* ref = akForm->AsReference()) {
			targ->target.handle = ref->CreateRefHandle();
			targ->targType = static_cast<std::int8_t>(TargetType::kSpecificReference);
		}
		else if (akForm->Is(RE::FormType::Keyword)) {
			targ->target.object = akForm;
			targ->targType = static_cast<std::int8_t>(TargetType::kKeyword);
		}
		else {
			targ->target.object = akForm;
			targ->targType = static_cast<std::int8_t>(TargetType::kObjectID);
		}
		return true;
	}

	//for target type 2 (object type):
	// kNone = 0,
	// kACTI = 1,
	// kARMO = 2,
	// kBOOK = 3,
	// kCONT = 4,
	// kDOOR = 5,
	// kINGR = 6,
	// kLIGH = 7,
	// kMISC = 8,
	// kFLOR = 9,
	// kFURN = 10,
	// kWEAP = 11,
	// kAMMO = 12,
	// kKEYM = 13,
	// kALCH = 14,
	// kFOOD = 15
	int GetPackageTargetObjectType(RE::StaticFunctionTag*, RE::TESPackage* akPackage) {
		auto* targ = gfuncs::IsFormValid(akPackage) ? GetPackageTargetPtr(akPackage) : nullptr;
		if (!targ || targ->targType != static_cast<std::int8_t>(TargetType::kObjectType)) {
			return -1;
		}
		return static_cast<int>(targ->target.objType.underlying());
	}

	//for target type 2 (object type):
	// kNone = 0,
	// kACTI = 1,
	// kARMO = 2,
	// kBOOK = 3,
	// kCONT = 4,
	// kDOOR = 5,
	// kINGR = 6,
	// kLIGH = 7,
	// kMISC = 8,
	// kFLOR = 9,
	// kFURN = 10,
	// kWEAP = 11,
	// kAMMO = 12,
	// kKEYM = 13,
	// kALCH = 14,
	// kFOOD = 15
	bool SetPackageTargetObjectType(RE::StaticFunctionTag*, RE::TESPackage* akPackage, int objType) {
		if (!gfuncs::IsFormValid(akPackage) || objType < 0 || objType > 15) {
			return false;
		}
		auto* targ = GetPackageTargetPtr(akPackage);
		if (!targ) {
			return false;
		}
		targ->target.object = nullptr;   //clear the full 8 bytes
		targ->target.objType = static_cast<RE::PACKAGE_OBJECT_TYPE>(objType);
		targ->targType = static_cast<std::int8_t>(TargetType::kObjectType);
		return true;
	}
	
	//BGSIdleCollection ==============================================================================================================================

	// kPickSequence = 1
	// kOldPickConditions = 2
	// kDoOnce = 3
	// kLooseOnly = 4
	// kNoSandbox = 5
	bool GetPackageIdleFlagSet(RE::StaticFunctionTag*, RE::TESPackage* akPackage, int flag) {
		if (!gfuncs::IsFormValid(akPackage) || !akPackage->idleCollection || !IndexInRange(flag, kIdleFlags, "idle flag")) {
			return false;
		}
		return akPackage->idleCollection->idleFlags.any(kIdleFlags[flag]);
	}

	void SetPackageIdleFlag(RE::StaticFunctionTag*, RE::TESPackage* akPackage, int flag, bool set) {
		if (!gfuncs::IsFormValid(akPackage) || !akPackage->idleCollection || !IndexInRange(flag, kIdleFlags, "idle flag")) {
			return;
		}
		if (set) {
			akPackage->idleCollection->idleFlags.set(kIdleFlags[flag]);
		}
		else {
			akPackage->idleCollection->idleFlags.reset(kIdleFlags[flag]);
		}
	}

	int GetPackageIdleCount(RE::StaticFunctionTag*, RE::TESPackage* akPackage) {
		if (!gfuncs::IsFormValid(akPackage) || !akPackage->idleCollection) {
			return 0;
		}
		return static_cast<int>(akPackage->idleCollection->idleCount);
	}

	RE::TESIdleForm* GetNthPackageIdle(RE::StaticFunctionTag*, RE::TESPackage* akPackage, int index) {
		if (!gfuncs::IsFormValid(akPackage) || !akPackage->idleCollection) {
			return nullptr;
		}
		auto* idles = akPackage->idleCollection;
		if (!idles->idles || index < 0 || index >= idles->idleCount) {
			return nullptr;
		}
		return idles->idles[index];
	}

	//-1 if not found
	int GetPackageIdleIndex(RE::StaticFunctionTag*, RE::TESPackage* akPackage, RE::TESIdleForm* akIdle) {
		if (!gfuncs::IsFormValid(akPackage) || !akPackage->idleCollection || !gfuncs::IsFormValid(akIdle)) {
			return -1;
		}
		auto index = akPackage->idleCollection->GetIndex(akIdle);
		return index ? static_cast<int>(*index) : -1;
	}

	//creates the package's idle collection if it doesn't have one
	bool AddPackageIdle(RE::StaticFunctionTag*, RE::TESPackage* akPackage, RE::TESIdleForm* akIdle) {
		if (!gfuncs::IsFormValid(akPackage) || !gfuncs::IsFormValid(akIdle)) {
			return false;
		}
		if (!akPackage->idleCollection) {
			akPackage->idleCollection = RE::BGSIdleCollection::Create();
			if (!akPackage->idleCollection) {
				return false;
			}
		}
		return akPackage->idleCollection->AddIdle(akIdle);
	}

	bool RemovePackageIdle(RE::StaticFunctionTag*, RE::TESPackage* akPackage, RE::TESIdleForm* akIdle) {
		if (!gfuncs::IsFormValid(akPackage) || !akPackage->idleCollection || !gfuncs::IsFormValid(akIdle)) {
			return false;
		}
		return akPackage->idleCollection->RemoveIdle(akIdle);
	}

	//PackageSchedule ================================================================================================================================
	//-1 means "any" for month, day of week, hour and minute. 0 means "any" for date.

	int GetPackageScheduleMonth(RE::StaticFunctionTag*, RE::TESPackage* akPackage) {
		return gfuncs::IsFormValid(akPackage) ? akPackage->packSched.psData.month : -1;
	}

	//-1 (any) or 0 to 11
	void SetPackageScheduleMonth(RE::StaticFunctionTag*, RE::TESPackage* akPackage, int month) {
		if (!gfuncs::IsFormValid(akPackage) || month < -1 || month > 11) {
			return;
		}
		akPackage->packSched.psData.month = static_cast<std::int8_t>(month);
	}

	// -1 any, 0 Sunday ... 6 Saturday, 7 weekdays, 8 weekends, 9 Monday/Wednesday/Friday, 10 Tuesday/Thursday
	int GetPackageScheduleDayOfWeek(RE::StaticFunctionTag*, RE::TESPackage* akPackage) {
		if (!gfuncs::IsFormValid(akPackage)) {
			return -1;
		}
		return static_cast<int>(akPackage->packSched.psData.dayOfWeek.underlying());
	}

	void SetPackageScheduleDayOfWeek(RE::StaticFunctionTag*, RE::TESPackage* akPackage, int day) {
		if (!gfuncs::IsFormValid(akPackage) || day < -1 || day > 10) {
			return;
		}
		akPackage->packSched.psData.dayOfWeek = static_cast<RE::PACK_SCHED_DATA::DayOfWeek>(day);
	}

	int GetPackageScheduleDate(RE::StaticFunctionTag*, RE::TESPackage* akPackage) {
		return gfuncs::IsFormValid(akPackage) ? akPackage->packSched.psData.date : 0;
	}

	//0 (any) or 1 to 31
	void SetPackageScheduleDate(RE::StaticFunctionTag*, RE::TESPackage* akPackage, int date) {
		if (!gfuncs::IsFormValid(akPackage) || date < 0 || date > 31) {
			return;
		}
		akPackage->packSched.psData.date = static_cast<std::int8_t>(date);
	}

	int GetPackageScheduleHour(RE::StaticFunctionTag*, RE::TESPackage* akPackage) {
		return gfuncs::IsFormValid(akPackage) ? akPackage->packSched.psData.hour : -1;
	}

	//-1 (any) or 0 to 23
	void SetPackageScheduleHour(RE::StaticFunctionTag*, RE::TESPackage* akPackage, int hour) {
		if (!gfuncs::IsFormValid(akPackage) || hour < -1 || hour > 23) {
			return;
		}
		akPackage->packSched.psData.hour = static_cast<std::int8_t>(hour);
	}

	int GetPackageScheduleMinute(RE::StaticFunctionTag*, RE::TESPackage* akPackage) {
		return gfuncs::IsFormValid(akPackage) ? akPackage->packSched.psData.minute : -1;
	}

	//-1 (any) or 0 to 59
	void SetPackageScheduleMinute(RE::StaticFunctionTag*, RE::TESPackage* akPackage, int minute) {
		if (!gfuncs::IsFormValid(akPackage) || minute < -1 || minute > 59) {
			return;
		}
		akPackage->packSched.psData.minute = static_cast<std::int8_t>(minute);
	}

	//in minutes
	int GetPackageScheduleDuration(RE::StaticFunctionTag*, RE::TESPackage* akPackage) {
		return gfuncs::IsFormValid(akPackage) ? akPackage->packSched.psData.duration : 0;
	}

	void SetPackageScheduleDuration(RE::StaticFunctionTag*, RE::TESPackage* akPackage, int minutes) {
		if (!gfuncs::IsFormValid(akPackage) || minutes < 0) {
			return;
		}
		akPackage->packSched.psData.duration = minutes;
	}

	//combat style, owner quest, procedure type =====================================================================================================

	RE::TESCombatStyle* GetPackageCombatStyle(RE::StaticFunctionTag*, RE::TESPackage* akPackage) {
		return gfuncs::IsFormValid(akPackage) ? akPackage->combatStyle : nullptr;
	}

	//none clears it
	void SetPackageCombatStyle(RE::StaticFunctionTag*, RE::TESPackage* akPackage, RE::TESCombatStyle* akStyle) {
		if (!gfuncs::IsFormValid(akPackage)) {
			return;
		}
		akPackage->combatStyle = akStyle;
	}

	//none clears it
	void SetPackageOwningQuest(RE::StaticFunctionTag*, RE::TESPackage* akPackage, RE::TESQuest* akQuest) {
		if (!gfuncs::IsFormValid(akPackage)) {
			return;
		}
		akPackage->ownerQuest = akQuest;
	}

	//see PACKAGE_PROCEDURE_TYPE in TESPackage.h, -1 to 48
	int GetPackageProcedureType(RE::StaticFunctionTag*, RE::TESPackage* akPackage) {
		if (!gfuncs::IsFormValid(akPackage)) {
			return -1;
		}
		return static_cast<int>(static_cast<std::int32_t>(akPackage->procedureType.underlying()));
	}
	
	std::string GetPackageProcedureTypeString(RE::StaticFunctionTag*, RE::TESPackage* akPackage) {
		if (!gfuncs::IsFormValid(akPackage)) {
			return "";
		}
		auto type = akPackage->procedureType.get();
		return static_cast<std::string>(magic_enum::enum_name(type));
	}

	//Changes the procedure type only, not the package's procedure tree / template data.
	void SetPackageProcedureType(RE::StaticFunctionTag*, RE::TESPackage* akPackage, int type) {
		if (!gfuncs::IsFormValid(akPackage)) {
			return;
		}
		if (type < -1 || type > 48) {
			logger::error("procedure type [{}] is out of range, must be -1 to 48", type);
			return;
		}
		akPackage->procedureType = static_cast<RE::PACKAGE_PROCEDURE_TYPE>(type);
	}

	bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
		//PACKAGE_DATA
		vm->RegisterFunction("GetPackageFlagSet", "DbSkseFunctions", GetPackageFlagSet);
		vm->RegisterFunction("SetPackageFlag", "DbSkseFunctions", SetPackageFlag);
		vm->RegisterFunction("GetPackagePreferredSpeed", "DbSkseFunctions", GetPackagePreferredSpeed);
		vm->RegisterFunction("SetPackagePreferredSpeed", "DbSkseFunctions", SetPackagePreferredSpeed);
		vm->RegisterFunction("GetPackageInterruptFlag", "DbSkseFunctions", GetPackageInterruptFlag);
		vm->RegisterFunction("SetPackageInterruptFlag", "DbSkseFunctions", SetPackageInterruptFlag);
		vm->RegisterFunction("GetPackageInterruptOverrideType", "DbSkseFunctions", GetPackageInterruptOverrideType);
		vm->RegisterFunction("SetPackageInterruptOverrideType", "DbSkseFunctions", SetPackageInterruptOverrideType);

		//PackageLocation
		vm->RegisterFunction("GetPackageLocationType", "DbSkseFunctions", GetPackageLocationType);
		vm->RegisterFunction("SetPackageLocationType", "DbSkseFunctions", SetPackageLocationType);
		vm->RegisterFunction("GetPackageLocationForm", "DbSkseFunctions", GetPackageLocationForm);
		vm->RegisterFunction("SetPackageLocationForm", "DbSkseFunctions", SetPackageLocationForm);
		vm->RegisterFunction("GetPackageLocationRef", "DbSkseFunctions", GetPackageLocationRef);
		vm->RegisterFunction("SetPackageLocationRef", "DbSkseFunctions", SetPackageLocationRef);
		vm->RegisterFunction("GetPackageLocationRadius", "DbSkseFunctions", GetPackageLocationRadius);
		vm->RegisterFunction("SetPackageLocationRadius", "DbSkseFunctions", SetPackageLocationRadius);

		//PackageTarget
		vm->RegisterFunction("GetPackageTargetType", "DbSkseFunctions", GetPackageTargetType);
		vm->RegisterFunction("GetPackageTarget", "DbSkseFunctions", GetPackageTarget);
		vm->RegisterFunction("SetPackageTarget", "DbSkseFunctions", SetPackageTarget);
		vm->RegisterFunction("GetPackageTargetObjectType", "DbSkseFunctions", GetPackageTargetObjectType);
		vm->RegisterFunction("SetPackageTargetObjectType", "DbSkseFunctions", SetPackageTargetObjectType);

		//BGSIdleCollection
		vm->RegisterFunction("GetPackageIdleFlagSet", "DbSkseFunctions", GetPackageIdleFlagSet);
		vm->RegisterFunction("SetPackageIdleFlag", "DbSkseFunctions", SetPackageIdleFlag);
		vm->RegisterFunction("GetPackageIdleCount", "DbSkseFunctions", GetPackageIdleCount);
		vm->RegisterFunction("GetNthPackageIdle", "DbSkseFunctions", GetNthPackageIdle);
		vm->RegisterFunction("GetPackageIdleIndex", "DbSkseFunctions", GetPackageIdleIndex);
		vm->RegisterFunction("AddPackageIdle", "DbSkseFunctions", AddPackageIdle);
		vm->RegisterFunction("RemovePackageIdle", "DbSkseFunctions", RemovePackageIdle);

		//PackageSchedule
		vm->RegisterFunction("GetPackageScheduleMonth", "DbSkseFunctions", GetPackageScheduleMonth);
		vm->RegisterFunction("SetPackageScheduleMonth", "DbSkseFunctions", SetPackageScheduleMonth);
		vm->RegisterFunction("GetPackageScheduleDayOfWeek", "DbSkseFunctions", GetPackageScheduleDayOfWeek);
		vm->RegisterFunction("SetPackageScheduleDayOfWeek", "DbSkseFunctions", SetPackageScheduleDayOfWeek);
		vm->RegisterFunction("GetPackageScheduleDate", "DbSkseFunctions", GetPackageScheduleDate);
		vm->RegisterFunction("SetPackageScheduleDate", "DbSkseFunctions", SetPackageScheduleDate);
		vm->RegisterFunction("GetPackageScheduleHour", "DbSkseFunctions", GetPackageScheduleHour);
		vm->RegisterFunction("SetPackageScheduleHour", "DbSkseFunctions", SetPackageScheduleHour);
		vm->RegisterFunction("GetPackageScheduleMinute", "DbSkseFunctions", GetPackageScheduleMinute);
		vm->RegisterFunction("SetPackageScheduleMinute", "DbSkseFunctions", SetPackageScheduleMinute);
		vm->RegisterFunction("GetPackageScheduleDuration", "DbSkseFunctions", GetPackageScheduleDuration);
		vm->RegisterFunction("SetPackageScheduleDuration", "DbSkseFunctions", SetPackageScheduleDuration);

		//other
		vm->RegisterFunction("GetPackageCombatStyle", "DbSkseFunctions", GetPackageCombatStyle);
		vm->RegisterFunction("SetPackageCombatStyle", "DbSkseFunctions", SetPackageCombatStyle);
		vm->RegisterFunction("SetPackageOwningQuest", "DbSkseFunctions", SetPackageOwningQuest);
		vm->RegisterFunction("GetPackageProcedureType", "DbSkseFunctions", GetPackageProcedureType);
		vm->RegisterFunction("GetPackageProcedureTypeString", "DbSkseFunctions", GetPackageProcedureTypeString);
		// vm->RegisterFunction("SetPackageProcedureType", "DbSkseFunctions", SetPackageProcedureType);

		return true;
	}
}
