#pragma once 

extern int eventPollingInterval; //in milliseconds

namespace rangeEvents {
	extern std::mutex updateMutex;
	extern std::condition_variable updateCv;
	extern bool isEmpty;
	bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm);
}
