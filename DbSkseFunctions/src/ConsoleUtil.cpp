#include "ConsoleUtil.h"
#include "GeneralFunctions.h"

namespace ConsoleUtil {
    const std::string consolePath = "_global.Console.ConsoleInstance";
    const std::string consoleCommandEntryPath = "_global.Console.ConsoleInstance.CommandEntry.text";
    const std::string consoleCommandHistoryPath = "_global.Console.ConsoleInstance.CommandHistory.text";

    std::string GetCommandEntryText() {
        std::string s = "";
        auto* ui = RE::UI::GetSingleton();
        if (!ui) {
            logger::error("ui not found");
            return s;
        }

        auto menu = ui->GetMenu<RE::Console>();
        if (menu) {
            RE::GFxValue gfx;
            if (menu->uiMovie->GetVariable(&gfx, consoleCommandEntryPath.c_str())) {
                if (gfx.IsString()) {
                    s = gfx.GetString();
                }
            }
        }

        return s;
    }

    bool SetCommandEntryText(std::string command, bool openConsole, bool execute, bool setInvisible) {
        logger::info("command[{}] execute[{}]", command, execute);

        if (command == "") {
            logger::error("empty command");
            return false;
        }

        auto* ui = RE::UI::GetSingleton();
        if (!ui) {
            logger::error("ui not found");
            return false;
        }

        auto menu = ui->GetMenu<RE::Console>();
        if (!menu) {
            logger::info("console menu not found");
            return false;
        }

        if (setInvisible && execute) {
            menu->uiMovie->SetVisible(false);
        } 

        if (openConsole) {
            if (!ui->IsMenuOpen(RE::Console::MENU_NAME)) {
                auto* msgQueue = RE::UIMessageQueue::GetSingleton();
                if (!msgQueue) {
                    logger::info("msgQueue not found");
                    return false;
                }

                int i = 0;
                msgQueue->AddMessage(RE::Console::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr); //open console
                while (!ui->IsMenuOpen(RE::Console::MENU_NAME) && i < 100) { //
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    i++;
                }
            }
        }

        menu->uiMovie->SetVariable(ConsoleUtil::consoleCommandEntryPath.c_str(), command.c_str());

        if (execute && ui->IsMenuOpen(RE::Console::MENU_NAME)) {
            RE::GFxKeyEvent enterKeyDown;
            enterKeyDown.keyCode = RE::GFxKey::Code::kKP_Enter;
            enterKeyDown.type = REX::EnumSet<RE::GFxEvent::EventType, std::uint32_t>{ RE::GFxEvent::EventType::kKeyDown };
            enterKeyDown.asciiCode = 28;
            menu->uiMovie->HandleEvent(enterKeyDown); //execute command directly from the console menu
        }

        if (setInvisible && execute) {
            menu->uiMovie->SetVisible(true);
        }
        return true;
    }

    std::string GetCommandHistoryText() {
        std::string s = "";
        auto* ui = RE::UI::GetSingleton();
        if (!ui) {
            logger::error("ui not found");
            return s;
        }

        auto menu = ui->GetMenu<RE::Console>();
        if (menu) {
            RE::GFxValue gfx;
            if (menu->uiMovie->GetVariable(&gfx, consoleCommandHistoryPath.c_str())) {
                if (gfx.IsString()) {
                    s = gfx.GetString();
                }
            }
        }

        return s;
    }

    void SetCommandHistoryText(std::string text) {
        auto* ui = RE::UI::GetSingleton();
        if (!ui) {
            logger::error("ui not found");
            return;
        }

        auto menu = ui->GetMenu<RE::Console>();
        if (menu) {
            menu->uiMovie->SetVariable(consoleCommandHistoryPath.c_str(), text.c_str());
        }
    }

    void _RemoveErrorMessageFromCommandHistory(std::string command, size_t milliSecondWait) {
        std::string errorMsg = "Script command \"" + command + "\" not found.";

        if (milliSecondWait > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(milliSecondWait));
        }
        std::string historyText = GetCommandHistoryText();
        int size = errorMsg.size();
        std::size_t index = historyText.find(errorMsg);
        //logger::info("removing errorMsg[{}]. Size[{}] index[{}]. Before history\n[{}]", errorMsg, size, index, historyText);
        //logger::info("removing errorMsg[{}]. Size[{}] index[{}]", errorMsg, size, index);

        if (index != std::string::npos) {
            //logger::info("removing errorMsg[{}] found", errorMsg);
            historyText.replace(index, (size + 1), "");
            SetCommandHistoryText(historyText);
            //logger::info("removing errorMsg[{}]. Size[{}] index[{}]. After history\n[{}]", errorMsg, size, index, historyText);
        }
        //else {
            //logger::info("removing errorMsg[{}] not found", errorMsg);
        //}
    }

    void RemoveErrorMessageFromCommandHistory(std::string command, size_t milliSecondWait, bool newThread) {
        //remove the 'script command "akCommand" not found' from history
        if (newThread) {
            std::thread t(_RemoveErrorMessageFromCommandHistory, command, milliSecondWait);
            t.detach();
        }
        else {
            _RemoveErrorMessageFromCommandHistory(command, milliSecondWait);
        }
    }
	
    //edited from ConsoleUtil NG
	void ExecuteCommand(std::string a_command, RE::TESObjectREFR* objRef) {
		const auto scriptFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
		const auto script = scriptFactory ? scriptFactory->Create() : nullptr;
		if (script) {
			script->SetCommand(a_command); 
			RE::TESObjectREFR* ref = nullptr;
			
			if (gfuncs::IsFormValid(objRef)) {
				ref = objRef;
			} 
			else {
				const auto selectedRef = RE::Console::GetSelectedRef();
				if (selectedRef) {
					ref = selectedRef.get();
					if (!gfuncs::IsFormValid(ref)) {
						ref = nullptr;
					}
				}
			}
			
			script->CompileAndRun(ref);
			delete script;
		}
	} 
}
