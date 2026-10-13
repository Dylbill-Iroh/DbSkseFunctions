#pragma once
	
namespace conditions {
	
    extern std::condition_variable updateCv;
    extern bool isEmpty;
	
	void LogAllConditionFunctionParamTypes();
	
	namespace Serialization {
		constexpr std::uint32_t conditionsRecord = 'DCnd';
		bool Save(SKSE::SerializationInterface* ssi);
		bool Load(SKSE::SerializationInterface* ssi, std::uint32_t version);
	}
	
    bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm);
}
