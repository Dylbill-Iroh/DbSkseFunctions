#include "KeyInput.h"
#include "GeneralFunctions.h"

namespace input {
    RE::INPUT_DEVICE GetInputDeviceForKeyCode(int keyCode) {
        if (keyCode <= 211) {
            return RE::INPUT_DEVICE::kKeyboard;
        }
        else if (keyCode <= 265) {
            return RE::INPUT_DEVICE::kMouse;
        }
        else {
            return RE::INPUT_DEVICE::kGamepad;
        }
    }

    void TapKeyPapyrus(int keyCode) {
        auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
        if (vm) {
            std::string testDebug = " messageBox from c++";
            auto* args = RE::MakeFunctionArguments((int)keyCode);
            RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> result;
            result.reset();
            //callBack.get()->SetObject()
            vm->DispatchStaticCall("Input", "TapKey", args, result);
        }
    }

    void TapKey(int keyCode) {
        auto bsInputEventQueue = RE::BSInputEventQueue::GetSingleton();
        if (bsInputEventQueue) {
            auto inputDevice = GetInputDeviceForKeyCode(keyCode);
            static auto kEvent = RE::ButtonEvent::Create(inputDevice, "", keyCode, 1, 0.0f);
            bsInputEventQueue->PushOntoInputQueue(kEvent);
        }
        else {
            logger::error("bsInputEventQueue not found key[{}] not tapped", keyCode);
        }
    }

    void TapKey(std::string keyString) {
        auto bsInputEventQueue = RE::BSInputEventQueue::GetSingleton();
        if (bsInputEventQueue) {
            auto bsInputEventQueue = RE::BSInputEventQueue::GetSingleton();
            static auto kEvent = RE::ButtonEvent::Create(RE::INPUT_DEVICE::kKeyboard, keyString, -1, 1, 0.0f);
            bsInputEventQueue->PushOntoInputQueue(kEvent);
        }
        else {
            logger::error("bsInputEventQueue not found key[{}] not tapped", keyString);
        }
    }

    void HoldKey(int keyCode, int holdTimeMilliSeconds) {
        std::thread([=] {
            if (auto bsInputEventQueue = RE::BSInputEventQueue::GetSingleton()) {
                auto inputDevice = GetInputDeviceForKeyCode(keyCode);
                // Here we're attempting to emulate the button "press," pause (indicating "hold"), and "release"
                // sequence
                static auto kEvent1 = RE::ButtonEvent::Create(inputDevice, "", keyCode, 1, 0.0f);
                static auto kEvent2 = RE::ButtonEvent::Create(inputDevice, "", keyCode, 0, 0.0f);

                bsInputEventQueue->PushOntoInputQueue(kEvent1);
                std::this_thread::sleep_for(std::chrono::milliseconds(holdTimeMilliSeconds));
                bsInputEventQueue->PushOntoInputQueue(kEvent2);
            }
            else {
                logger::error("bsInputEventQueue not found key[{}] not tapped", keyCode);
            }
            }).detach();
    }

    void HoldKey(std::string keyString, int holdTimeMilliSeconds) {
        std::thread([=] {
            if (auto bsInputEventQueue = RE::BSInputEventQueue::GetSingleton()) {
                // Here we're attempting to emulate the button "press," pause (indicating "hold"), and "release"
                // sequence 

                static auto kEvent1 = RE::ButtonEvent::Create(RE::INPUT_DEVICE::kKeyboard, keyString, -1, 1, 0.0f);
                static auto kEvent2 = RE::ButtonEvent::Create(RE::INPUT_DEVICE::kKeyboard, keyString, -1, 0, 0.0f);

                bsInputEventQueue->PushOntoInputQueue(kEvent1);
                std::this_thread::sleep_for(std::chrono::milliseconds(holdTimeMilliSeconds));
                bsInputEventQueue->PushOntoInputQueue(kEvent2);
            }
            else { 
                logger::error("bsInputEventQueue not found key[{}] not tapped", keyString);
            }
            }).detach();
    }
}