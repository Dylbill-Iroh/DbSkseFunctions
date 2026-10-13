#pragma once 

namespace objectRef {
	constexpr std::uint32_t objectRefRecord = 'ObR1';
	bool SaveTestRef(SKSE::SerializationInterface* ssi);
	bool LoadTestRef(SKSE::SerializationInterface* ssi);
	
    bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm);
}