#include "Timers.h"
#include "GeneralFunctions.h"
#include "Utility.h"
#include "SharedVariables.h"

int gameTimerPollingInterval = 1500; //in milliseconds

//menu mode timer ===================================================================================================================================
//no restrictions on time, time is always counted. 

void EraseFinishedMenuModeTimers();
bool erasingMenuModeTimers = false;
RE::BSFixedString sMenuModeTimerEvent = "OnTimerMenuMode";

struct MenuModeTimer {
    std::chrono::system_clock::time_point startTime;
    RE::VMHandle handle;
    int timerID;
    float interval;
    float savedTimeElapsed = 0.0;
    bool cancelled = false;
    bool finished = false;

    MenuModeTimer(RE::VMHandle akHandle, float afInterval, int aiTimerID, float afSavedTimeElapsed = 0.0) {
        handle = akHandle;
        timerID = aiTimerID;
        interval = afInterval;
        savedTimeElapsed = afSavedTimeElapsed;

        std::thread t([=]() {
            startTime = std::chrono::system_clock::now();

            int milliSecondInterval = (interval * 1000);
            std::this_thread::sleep_for(std::chrono::milliseconds(milliSecondInterval));

            if (!cancelled) {
                if (sv::skyrimVm) {
                    float elapsedTime = (savedTimeElapsed + gfuncs::timePointDiffToFloat(std::chrono::system_clock::now(), startTime));
                    auto* args = RE::MakeFunctionArguments((int)timerID);
                    sv::skyrimVm->SendAndRelayEvent(handle, &sMenuModeTimerEvent, args, nullptr);
                    delete args;
                    logger::debug("menu mode timer event sent. ID[{}], interval[{}], elapsed time[{}]", timerID, interval, elapsedTime);
                }
                else {
                    logger::error("sv::skyrimVm* not found, timer[{}] for handle[{}] event not sent.", timerID, handle);
                }
            }

            finished = true;
            EraseFinishedMenuModeTimers();
            });
        t.detach();
    }

    float GetElapsedTime() {
        return (savedTimeElapsed + gfuncs::timePointDiffToFloat(std::chrono::system_clock::now(), startTime));
    }

    float GetCurrentElapsedTime() { //for current - after loading a save startTime and interval are reset.
        return gfuncs::timePointDiffToFloat(std::chrono::system_clock::now(), startTime);
    }

    float GetTimeLeft() {
        float timeLeft = (interval - GetCurrentElapsedTime());
        if (timeLeft < 0.0) {
            timeLeft = 0.0;
        }
        return (timeLeft);
    }
};

std::vector<MenuModeTimer*> currentMenuModeTimers;
std::mutex currentMenuModeTimersMutex;

void AddToCurrentMenuModeTimers(MenuModeTimer* timer) {
    std::lock_guard<std::mutex> lock(currentMenuModeTimersMutex);
    currentMenuModeTimers.push_back(timer);
}

void EraseFinishedMenuModeTimers() {
    std::lock_guard<std::mutex> lock(currentMenuModeTimersMutex);

    if (currentMenuModeTimers.size() == 0) {
        return;
    }

    erasingMenuModeTimers = true;

    for (int i = 0; i < currentMenuModeTimers.size(); i++) {
        auto* timer = currentMenuModeTimers[i];
        if (timer) {
            if (timer->finished) {
                delete timer;
                timer = nullptr;
                //logger::trace("erased menuModeTimer, Timers left = {}", currentMenuModeTimers.size());
            }
        }

        if (!timer) {
            auto it = currentMenuModeTimers.begin() + i;
            currentMenuModeTimers.erase(it);
            i--; //move i back 1 cause 1 timer was just erased
        }
    }
    erasingMenuModeTimers = false;
}

MenuModeTimer* GetTimer(std::vector<MenuModeTimer*>& v, RE::VMHandle akHandle, int aiTimerID) {
    std::lock_guard<std::mutex> lock(currentMenuModeTimersMutex);

    if (v.size() == 0) {
        return nullptr;
    }


    for (int i = 0; i < v.size(); i++) {
        auto* timer = v[i];
        if (timer) {
            if (!timer->cancelled && !timer->finished) {
                if (timer->handle == akHandle && timer->timerID == aiTimerID) {
                    return timer;
                }
            }
        }
    }

    return nullptr;
}

bool SaveTimers(std::vector<MenuModeTimer*>& v, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    std::lock_guard<std::mutex> lock(currentMenuModeTimersMutex);

    if (!a_intfc->OpenRecord(record, 1)) {
        logger::error("MenuModeTimers Failed to open record[{}]", record);
        return false;
    }

    const std::size_t size = v.size();

    if (!a_intfc->WriteRecordData(size)) {
        logger::error("record[{}] Failed to write size of MenuModeTimers!", record);
        return false;
    }

    for (auto* timer : v) {
        if (timer) {
            float timeLeft = timer->GetTimeLeft();
            float timeElapsed = timer->GetElapsedTime();

            if (!a_intfc->WriteRecordData(timeLeft)) {
                logger::error("record[{}] Failed to write time left", record);

                return false;
            }

            if (!a_intfc->WriteRecordData(timeElapsed)) {
                logger::error("record[{}] Failed to write time elapsed", record);

                return false;
            }

            if (!a_intfc->WriteRecordData(timer->handle)) {
                logger::error("record[{}] Failed to write handle[{}]", record, timer->handle);

                return false;
            }

            if (!a_intfc->WriteRecordData(timer->timerID)) {
                logger::error("record[{}] Failed to write timerID[{}]", record, timer->timerID);

                return false;
            }

            if (!a_intfc->WriteRecordData(timer->cancelled)) {
                logger::error("record[{}] Failed to write cancelled[{}]", record, timer->cancelled);

                return false;
            }

            if (!a_intfc->WriteRecordData(timer->finished)) {
                logger::error("record[{}] Failed to write finished[{}]", record, timer->finished);

                return false;
            }

            logger::debug("MenuModeTimer saved : timeLeft[{}], timeElapsed[{}], handle[{}], ID[{}], cancelled[{}], finished[{}]",
                timeLeft, timeElapsed, timer->handle, timer->timerID, timer->cancelled, timer->finished);
        }
        else {
            float noTimerTimeLeft = -1.0;
            float noTimerTimeElapsed = 0.0;
            RE::VMHandle handle;
            int ID = -1;
            bool cancelled = true;
            bool finished = true;

            if (!a_intfc->WriteRecordData(noTimerTimeLeft)) {
                logger::error("record[{}] Failed to write no Timer Time Left", record);

                return false;
            }

            if (!a_intfc->WriteRecordData(noTimerTimeElapsed)) {
                logger::error("record[{}] Failed to write no Timer Time Elapsed", record);

                return false;
            }

            if (!a_intfc->WriteRecordData(handle)) {
                logger::error("record[{}] Failed to write no timer handle", record);

                return false;
            }

            if (!a_intfc->WriteRecordData(ID)) {
                logger::error("record[{}] Failed to write no timer Id", record);

                return false;
            }

            if (!a_intfc->WriteRecordData(cancelled)) {
                logger::error("record[{}] Failed to write no timer cancelled[{}]", record, cancelled);

                return false;
            }

            if (!a_intfc->WriteRecordData(finished)) {
                logger::error("record[{}] Failed to write no timer finished[{}]", record, finished);

                return false;
            }
        }
    }

    return true;
}

bool loadTimers(std::vector<MenuModeTimer*>& v, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    std::lock_guard<std::mutex> lock(currentMenuModeTimersMutex);

    std::size_t size;
    if (!a_intfc->ReadRecordData(size)) {
        logger::error("record[{}] Failed to load size of MenuModeTimers!", record);
        return false;
    }

    if (v.size() > 0) { //cancel current timers
        for (auto* timer : v) {
            if (timer) {
                timer->cancelled = true;
            }
        }
    }

    for (std::size_t i = 0; i < size; i++) {
        float timeLeft = -1.0;
        float timeElapsed = 0.0;
        RE::VMHandle handle;
        int ID = -1;
        bool cancelled = false;
        bool finished = false;

        if (!a_intfc->ReadRecordData(timeLeft)) {
            logger::error("{}: MenuModeTimer Failed to load timeLeft!", i);

            return false;
        }

        if (!a_intfc->ReadRecordData(timeElapsed)) {
            logger::error("{}: MenuModeTimer Failed to load timeElapsed!", i);

            return false;
        }

        if (!a_intfc->ReadRecordData(handle)) {
            logger::error("{}: MenuModeTimer Failed to load handle!", i);

            return false;
        }

        if (!a_intfc->ReadRecordData(ID)) {
            logger::error("{}: MenuModeTimer Failed to load ID!", i);

            return false;
        }

        if (!a_intfc->ReadRecordData(cancelled)) {
            logger::error("{}: MenuModeTimer Failed to load cancelled!", i);

            return false;
        }

        if (!a_intfc->ReadRecordData(finished)) {
            logger::error("{}: MenuModeTimer Failed to load finished!", i);

            return false;
        }

        if (!a_intfc->ResolveHandle(handle, handle)) {
            logger::warn("{}: MenuModeTimer warning, failed to resolve handle[{}]", i, handle);
            continue;
        }

        if (cancelled || finished) {
            continue;
        }

        MenuModeTimer* timer = new MenuModeTimer(handle, timeLeft, ID, timeElapsed);
        v.push_back(timer);

        logger::debug("MenuModeTimer loaded: timeLeft[{}], timeElapsed[{}], handle[{}], ID[{}], cancelled[{}], finished[{}]",
            timeLeft, timeElapsed, timer->handle, timer->timerID, timer->cancelled, timer->finished);
    }

    return true;
}

//NoMenuModeTimer =========================================================================================================================================
void EraseFinishedNoMenuModeTimers();
bool erasingNoMenuModeTimers = false;
bool noMenuModeTimersEmpty = true;
RE::BSFixedString sNoMenuModeTimerEvent = "OnTimerNoMenuMode";

struct NoMenuModeTimer {
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point endTime;
    std::chrono::system_clock::time_point lastMenuCheck;
    bool lastMenuCheckSet = false;
    float timeToWait;
    RE::VMHandle handle;
    int timerID;
    bool cancelled = false;
    bool started = false;
    bool finished = false;
    bool canDelete = false;

    NoMenuModeTimer(RE::VMHandle akHandle, float afInterval, int aitimerID) {
        startTime = std::chrono::system_clock::now();
        std::chrono::milliseconds millisecondsToAdd(int(afInterval * 1000));
        endTime = startTime + millisecondsToAdd;

        timeToWait = afInterval;
        handle = akHandle;
        timerID = aitimerID;

        StartTimer();
    }

    void StartTimer() {
        started = true;
        noMenuModeTimersEmpty = false;

        std::thread t([=]() {
            bool inMenu = sv::inMenuMode;
            if (inMenu) {
                lastMenuCheck = std::chrono::system_clock::now();
            }

            {
                std::unique_lock<std::mutex> lock(sv::updateMutex);
                // waiting
                sv::updateCv.wait(lock, [=] {
                    return (inMenu != sv::inMenuMode || sv::currentTimePoint >= endTime);
                });
            }

            if (inMenu) { //started timer while in menu, add time spent in menu.
                auto now = std::chrono::system_clock::now();
                auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastMenuCheck);
                std::chrono::milliseconds millisecondsToAdd(milliseconds);
                startTime = startTime + millisecondsToAdd; //for getTimeLeft and getTimeElapsed functions
                endTime = endTime + millisecondsToAdd;
            } 

            if (!cancelled) {
                if (sv::currentTimePoint >= endTime) {
                    if (sv::skyrimVm) {
                        float elapsedTime = GetElapsedTime();
                        auto* args = RE::MakeFunctionArguments((int)timerID);
                        sv::skyrimVm->SendAndRelayEvent(handle, &sNoMenuModeTimerEvent, args, nullptr);
                        logger::debug("NoMenuModeTimer event sent: timerID[{}] timeToWait[{}] elapsedTime[{}]",
                            timerID, timeToWait, elapsedTime);

                        delete args;
                        finished = true;
                    }
                    else {
                        logger::error("sv::skyrimVm* not found, timer[{}] for handle[{}] event not sent.", timerID, handle);
                        finished = true;
                    }
                }
                else {
                    StartTimer();
                }
            }
            
            if (cancelled || finished) {
                canDelete = true;
                EraseFinishedNoMenuModeTimers();
            }
            });
        t.detach();
    }

    float GetElapsedTime() {
        if (sv::inMenuMode) {
            return (gfuncs::timePointDiffToFloat(lastMenuCheck, startTime));
        }
        else {
            return (gfuncs::timePointDiffToFloat(std::chrono::system_clock::now(), startTime));
        }
    }

    float GetTimeLeft() {
        float elapsedTime = GetElapsedTime();
        return (timeToWait - elapsedTime);
    }
};

std::vector<NoMenuModeTimer*> currentNoMenuModeTimers;
std::mutex currentNoMenuModeTimersMutex;

void AddToCurrentNoMenuModeTimers(NoMenuModeTimer* timer) {
    std::lock_guard<std::mutex> lock(currentNoMenuModeTimersMutex);
    currentNoMenuModeTimers.push_back(timer);
}

void EraseFinishedNoMenuModeTimers() {
    std::lock_guard<std::mutex> lock(currentNoMenuModeTimersMutex);

    if (currentNoMenuModeTimers.size() == 0) {
        noMenuModeTimersEmpty = true;
        return;
    }

    erasingNoMenuModeTimers = true;

    for (int i = 0; i < currentNoMenuModeTimers.size(); i++) {
        auto* timer = currentNoMenuModeTimers[i];
        if (timer) {
            if (timer->finished) {
                delete timer;
                timer = nullptr;
                //logger::trace("erased noMenuModeTimer, Timers left = {}", currentNoMenuModeTimers.size());
            }
        }

        if (!timer) {
            auto it = currentNoMenuModeTimers.begin() + i;
            currentNoMenuModeTimers.erase(it);
            i--; //move i back 1 cause 1 timer was just erased
        }
    }

    if (currentNoMenuModeTimers.size() == 0) {
        noMenuModeTimersEmpty = true;
    }

    erasingNoMenuModeTimers = false;
}

NoMenuModeTimer* GetTimer(std::vector<NoMenuModeTimer*>& v, RE::VMHandle akHandle, int aiTimerID) {
    std::lock_guard<std::mutex> lock(currentNoMenuModeTimersMutex);

    if (v.size() == 0) {
        return nullptr;
    }

    for (int i = 0; i < v.size(); i++) {
        auto* timer = v[i];
        if (timer) {
            if (!timer->cancelled && !timer->finished) {
                if (timer->handle == akHandle && timer->timerID == aiTimerID) {
                    return timer;
                }
            }
        }
    }

    return nullptr;
}

bool SaveTimers(std::vector<NoMenuModeTimer*>& v, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    std::lock_guard<std::mutex> lock(currentNoMenuModeTimersMutex);

    if (!a_intfc->OpenRecord(record, 1)) {
        logger::error("MenuModeTimers Failed to open record[{}]", record);
        return false;
    }

    const std::size_t size = v.size();

    if (!a_intfc->WriteRecordData(size)) {
        logger::error("record[{}] Failed to write size of MenuModeTimers!", record);
        return false;
    }

    for (auto* timer : v) {
        if (timer) {
            float timeLeft = timer->GetTimeLeft();
            float timeElapsed = timer->GetElapsedTime();

            if (!a_intfc->WriteRecordData(timeLeft)) {
                logger::error("record[{}] Failed to write time left", record);

                return false;
            }

            if (!a_intfc->WriteRecordData(timeElapsed)) {
                logger::error("record[{}] Failed to write time elapsed", record);

                return false;
            }

            if (!a_intfc->WriteRecordData(timer->handle)) {
                logger::error("record[{}] Failed to write handle[{}]", record, timer->handle);

                return false;
            }

            if (!a_intfc->WriteRecordData(timer->timerID)) {
                logger::error("record[{}] Failed to write timerID[{}]", record, timer->timerID);

                return false;
            }

            if (!a_intfc->WriteRecordData(timer->cancelled)) {
                logger::error("record[{}] Failed to write cancelled[{}]", record, timer->cancelled);

                return false;
            }

            if (!a_intfc->WriteRecordData(timer->finished)) {
                logger::error("record[{}] Failed to write finished[{}]", record, timer->finished);

                return false;
            }

            logger::debug("NoMenuModeTimer saved : timeLeft[{}], timeElapsed[{}], handle[{}], ID[{}], cancelled[{}], finished[{}]",
                timeLeft, timeElapsed, timer->handle, timer->timerID, timer->cancelled, timer->finished);
        }
        else {
            float noTimerTimeLeft = -1.0;
            float noTimerTimeElapsed = 0.0;
            RE::VMHandle handle;
            int ID = -1;
            bool cancelled = true;
            bool finished = true;

            if (!a_intfc->WriteRecordData(noTimerTimeLeft)) {
                logger::error("record[{}] Failed to write no Timer Time Left", record);

                return false;
            }

            if (!a_intfc->WriteRecordData(noTimerTimeElapsed)) {
                logger::error("record[{}] Failed to write no Timer Time Elapsed", record);

                return false;
            }

            if (!a_intfc->WriteRecordData(handle)) {
                logger::error("record[{}] Failed to write no timer handle", record);

                return false;
            }

            if (!a_intfc->WriteRecordData(ID)) {
                logger::error("record[{}] Failed to write no timer Id", record);

                return false;
            }

            if (!a_intfc->WriteRecordData(cancelled)) {
                logger::error("record[{}] Failed to write no timer cancelled[{}]", record, cancelled);

                return false;
            }

            if (!a_intfc->WriteRecordData(finished)) {
                logger::error("record[{}] Failed to write no timer finished[{}]", record, finished);

                return false;
            }
        }
    }

    return true;
}

bool loadTimers(std::vector<NoMenuModeTimer*>& v, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    std::lock_guard<std::mutex> lock(currentNoMenuModeTimersMutex);

    std::size_t size;
    if (!a_intfc->ReadRecordData(size)) {
        logger::error("record[{}] Failed to load size of MenuModeTimers!", record);
        return false;
    }

    if (v.size() > 0) { //cancel current timers
        for (auto* timer : v) {
            if (timer) {
                timer->cancelled = true;
            }
        }
    }

    for (std::size_t i = 0; i < size; i++) {
        float timeLeft = -1.0;
        float timeElapsed = 0.0;
        RE::VMHandle handle;
        int ID = -1;
        bool cancelled = false;
        bool finished = false;

        if (!a_intfc->ReadRecordData(timeLeft)) {
            logger::error("{}: MenuModeTimer Failed to load timeLeft!", i);
            return false;
        }

        if (!a_intfc->ReadRecordData(timeElapsed)) {
            logger::error("{}: MenuModeTimer Failed to load timeElapsed!", i);
            return false;
        }

        if (!a_intfc->ReadRecordData(handle)) {
            logger::error("{}: MenuModeTimer Failed to load handle!", i);
            return false;
        }

        if (!a_intfc->ReadRecordData(ID)) {
            logger::error("{}: MenuModeTimer Failed to load ID!", i);
            return false;
        }

        if (!a_intfc->ReadRecordData(cancelled)) {
            logger::error("{}: MenuModeTimer Failed to load cancelled!", i);
            return false;
        }

        if (!a_intfc->ReadRecordData(finished)) {
            logger::error("{}: MenuModeTimer Failed to load finished!", i);
            return false;
        }

        if (!a_intfc->ResolveHandle(handle, handle)) {
            logger::warn("{}: MenuModeTimer warning, failed to resolve handle[{}]", i, handle);
            continue;
        }

        if (cancelled || finished) {
            continue;
        }

        NoMenuModeTimer* timer = new NoMenuModeTimer(handle, timeLeft, ID);
        v.push_back(timer);

        logger::debug("NoMenuModeTimer loaded: timeLeft[{}], timeElapsed[{}], handle[{}], ID[{}], cancelled[{}], finished[{}]",
            timeLeft, timeElapsed, timer->handle, timer->timerID, timer->cancelled, timer->finished);
    }

    return true;
}

//timer ===================================================================================================================================

void EraseFinishedTimers();
bool erasingTimers = false;
bool timersEmpty = true;
RE::BSFixedString sTimerEvent = "OnTimer";

struct Timer {
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point endTime;
    std::chrono::system_clock::time_point lastMenuCheck;
    bool lastMenuCheckSet = false;
    float timeToWait;
    RE::VMHandle handle;
    int timerID;
    bool cancelled = false;
    bool started = false;
    bool finished = false;
    bool canDelete = false;

    Timer(RE::VMHandle akHandle, float afInterval, int aitimerID) {
        startTime = std::chrono::system_clock::now();
        std::chrono::milliseconds millisecondsToAdd(int(afInterval * 1000));
        endTime = startTime + millisecondsToAdd;

        timeToWait = afInterval;
        handle = akHandle;
        timerID = aitimerID;

        StartTimer();
    }

    void StartTimer() {
        started = true;
        timersEmpty = false;

        std::thread t([=]() {
            
            bool gamePaused = sv::gamePaused;
            if (gamePaused) {
                lastMenuCheck = std::chrono::system_clock::now();
            }

            {
                std::unique_lock<std::mutex> lock(sv::updateMutex);
                // waiting
                sv::updateCv.wait(lock, [=] {
                    return (gamePaused != sv::gamePaused || sv::currentTimePoint >= endTime);
                });
            }

            if (gamePaused) { //started timer while in menu, add time spent in menu.
                auto now = std::chrono::system_clock::now();
                auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastMenuCheck);
                std::chrono::milliseconds millisecondsToAdd(milliseconds);
                startTime = startTime + millisecondsToAdd; //for getTimeLeft and getTimeElapsed functions
                endTime = endTime + millisecondsToAdd;
            }

            if (!cancelled) {
                if (sv::currentTimePoint >= endTime) {
                    if (sv::skyrimVm) {
                        float elapsedTime = GetElapsedTime();
                        auto* args = RE::MakeFunctionArguments((int)timerID);
                        sv::skyrimVm->SendAndRelayEvent(handle, &sTimerEvent, args, nullptr);
                        logger::debug("Timer event sent: timerID[{}] timeToWait[{}] elapsedTime[{}]",
                            timerID, timeToWait, elapsedTime);

                        delete args;
                        finished = true;
                    }
                    else {
                        logger::error("sv::skyrimVm* not found, timer[{}] for handle[{}] event not sent.", timerID, handle);
                        finished = true;
                    }
                }
                else {
                    StartTimer();
                }
            }

            if (cancelled || finished) {
                canDelete = true;
                EraseFinishedTimers();
            }
            });
        t.detach();
    }

    float GetElapsedTime() {
        if (sv::gamePaused) {
            return (gfuncs::timePointDiffToFloat(lastMenuCheck, startTime));
        }
        else {
            return (gfuncs::timePointDiffToFloat(std::chrono::system_clock::now(), startTime));
        }
    }

    float GetTimeLeft() {
        float elapsedTime = GetElapsedTime();
        return (timeToWait - elapsedTime);
    }
};

std::vector<Timer*> currentTimers;
std::mutex currentTimersMutex;

void AddToCurrentTimers(Timer* timer) {
    std::lock_guard<std::mutex> lock(currentTimersMutex);
    currentTimers.push_back(timer);
}

void EraseFinishedTimers() {
    std::lock_guard<std::mutex> lock(currentTimersMutex);

    if (currentTimers.size() == 0) {
        timersEmpty = true;
        return;
    }

    erasingTimers = true;

    for (int i = 0; i < currentTimers.size(); i++) {
        auto* timer = currentTimers[i];
        if (timer) {
            if (timer->finished) {
                delete timer;
                timer = nullptr;
                //logger::trace("erased Timer, Timers left = {}", currentTimers.size());
            }
        }

        if (!timer) {
            auto it = currentTimers.begin() + i;
            currentTimers.erase(it);
            i--; //move i back 1 cause 1 timer was just erased
        }
    }

    if (currentTimers.size() == 0) {
        timersEmpty = true;
    }

    erasingTimers = false;
}

Timer* GetTimer(std::vector<Timer*>& v, RE::VMHandle akHandle, int aiTimerID) {
    std::lock_guard<std::mutex> lock(currentTimersMutex);

    if (v.size() == 0) {
        return nullptr;
    }

    for (int i = 0; i < v.size(); i++) {
        auto* timer = v[i];
        if (timer) {
            if (!timer->cancelled && !timer->finished) {
                if (timer->handle == akHandle && timer->timerID == aiTimerID) {
                    return timer;
                }
            }
        }
    }

    return nullptr;
}

bool SaveTimers(std::vector<Timer*>& v, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    std::lock_guard<std::mutex> lock(currentTimersMutex);

    if (!a_intfc->OpenRecord(record, 1)) {
        logger::error("MenuModeTimers Failed to open record[{}]", record);
        return false;
    }

    const std::size_t size = v.size();

    if (!a_intfc->WriteRecordData(size)) {
        logger::error("record[{}] Failed to write size of MenuModeTimers!", record);
        return false;
    }

    for (auto* timer : v) {
        if (timer) {
            float timeLeft = timer->GetTimeLeft();
            float timeElapsed = timer->GetElapsedTime();

            if (!a_intfc->WriteRecordData(timeLeft)) {
                logger::error("record[{}] Failed to write time left", record);
                return false;
            }

            if (!a_intfc->WriteRecordData(timeElapsed)) {
                logger::error("record[{}] Failed to write time elapsed", record);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->handle)) {
                logger::error("record[{}] Failed to write handle[{}]", record, timer->handle);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->timerID)) {
                logger::error("record[{}] Failed to write timerID[{}]", record, timer->timerID);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->cancelled)) {
                logger::error("record[{}] Failed to write cancelled[{}]", record, timer->cancelled);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->finished)) {
                logger::error("record[{}] Failed to write finished[{}]", record, timer->finished);
                return false;
            }

            logger::debug("Timer saved : timeLeft[{}], timeElapsed[{}], handle[{}], ID[{}], cancelled[{}], finished[{}]",
                timeLeft, timeElapsed, timer->handle, timer->timerID, timer->cancelled, timer->finished);
        }
        else {
            float noTimerTimeLeft = -1.0;
            float noTimerTimeElapsed = 0.0;
            RE::VMHandle handle;
            int ID = -1;
            bool cancelled = true;
            bool finished = true;

            if (!a_intfc->WriteRecordData(noTimerTimeLeft)) {
                logger::error("record[{}] Failed to write no Timer Time Left", record);
                return false;
            }

            if (!a_intfc->WriteRecordData(noTimerTimeElapsed)) {
                logger::error("record[{}] Failed to write no Timer Time Elapsed", record);
                return false;
            }

            if (!a_intfc->WriteRecordData(handle)) {
                logger::error("record[{}] Failed to write no timer handle", record);
                return false;
            }

            if (!a_intfc->WriteRecordData(ID)) {
                logger::error("record[{}] Failed to write no timer Id", record);
                return false;
            }

            if (!a_intfc->WriteRecordData(cancelled)) {
                logger::error("record[{}] Failed to write no timer cancelled[{}]", record, cancelled);
                return false;
            }

            if (!a_intfc->WriteRecordData(finished)) {
                logger::error("record[{}] Failed to write no timer finished[{}]", record, finished);
                return false;
            }
        }
    }
    return true;
}

bool loadTimers(std::vector<Timer*>& v, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    std::lock_guard<std::mutex> lock(currentTimersMutex);

    std::size_t size;
    if (!a_intfc->ReadRecordData(size)) {
        logger::error("record[{}] Failed to load size of MenuModeTimers!", record);
        return false;
    }

    if (v.size() > 0) { //cancel current timers
        for (auto* timer : v) {
            if (timer) {
                timer->cancelled = true;
            }
        }
    }

    for (std::size_t i = 0; i < size; i++) {
        float timeLeft = -1.0;
        float timeElapsed = 0.0;
        RE::VMHandle handle;
        int ID = -1;
        bool cancelled = false;
        bool finished = false;

        if (!a_intfc->ReadRecordData(timeLeft)) {
            logger::error("{}: MenuModeTimer Failed to load timeLeft!", i);
            return false;
        }

        if (!a_intfc->ReadRecordData(timeElapsed)) {
            logger::error("{}: MenuModeTimer Failed to load timeElapsed!", i);
            return false;
        }

        if (!a_intfc->ReadRecordData(handle)) {
            logger::error("{}: MenuModeTimer Failed to load handle!", i);
            return false;
        }

        if (!a_intfc->ReadRecordData(ID)) {
            logger::error("{}: MenuModeTimer Failed to load ID!", i);
            return false;
        }

        if (!a_intfc->ReadRecordData(cancelled)) {
            logger::error("{}: MenuModeTimer Failed to load cancelled!", i);
            return false;
        }

        if (!a_intfc->ReadRecordData(finished)) {
            logger::error("{}: MenuModeTimer Failed to load finished!", i);
            return false;
        }

        if (!a_intfc->ResolveHandle(handle, handle)) {
            logger::warn("{}: MenuModeTimer warning, failed to resolve handle[{}]", i, handle);
            continue;
        }

        if (cancelled || finished) {
            continue;
        }

        Timer* timer = new Timer(handle, timeLeft, ID);
        v.push_back(timer);

        logger::debug("Timer loaded: timeLeft[{}], timeElapsed[{}], handle[{}], ID[{}], cancelled[{}], finished[{}]",
            timeLeft, timeElapsed, timer->handle, timer->timerID, timer->cancelled, timer->finished);
    }
    return true;
}


//GameTimeTimer =========================================================================================================================================

void EraseFinishedGameTimers();
bool erasingGameTimers = false;
RE::BSFixedString sGameTimeTimerEvent = "OnTimerGameTime";
bool gameTimersEmpty = true;

struct GameTimeTimer {
    float startTime;
    float endTime;
    int tick;
    int timerID;
    RE::VMHandle handle;
    bool cancelled = false;
    bool started = false;
    bool finished = false;

    GameTimeTimer(RE::VMHandle akHandle, float afInterval, int aiTimerID) {
        handle = akHandle;
        timerID = aiTimerID;

        if (sv::calendar) {
            endTime = sv::calendar->GetHoursPassed() + afInterval;
            StartTimer();
        }
        else {
            logger::error("sv::calendar* not found, timer[{}] for handle[{}] event not started.", timerID, handle);
            cancelled = true;
        }
    }

    void StartTimer() {
        gameTimersEmpty = false;

        std::thread t([=]() {
            {
                std::unique_lock<std::mutex> lock(sv::updateMutex);
                // waiting
                sv::updateCv.wait(lock, [=] {
                    return (sv::gameTime >= endTime);
                });
            }

            if (!cancelled) {
                auto now = std::chrono::system_clock::now();
                float endTime = sv::gameTime;
                float elapsedGameHours = (sv::gameTime - startTime);

                auto* args = RE::MakeFunctionArguments((int)timerID);
                sv::skyrimVm->SendAndRelayEvent(handle, &sGameTimeTimerEvent, args, nullptr);
                delete args;
                logger::debug("game timer event sent. ID[{}] startTime[{}] endTime[{}] elapsedGameHours[{}]",
                    timerID, startTime, endTime, elapsedGameHours);

                finished = true;
            }

            if (cancelled || finished) {
                EraseFinishedGameTimers();
            }
        });
        t.detach();
    }

    float GetElapsedTime() {
        if (sv::calendar) {
            return (sv::calendar->GetHoursPassed() - startTime);
        }
        else {
            logger::error("sv::calendar* not found, timer[{}] for handle[{}]", timerID, handle);
            return -1.0;
        }
    }

    float GetTimeLeft() {
        if (sv::calendar) {
            return (endTime - sv::calendar->GetHoursPassed());
        }
        else {
            logger::error("sv::calendar* not found, timer[{}] for handle[{}]", timerID, handle);
            return -1.0;
        }
    }

    void CancelTimer() {
        cancelled = true;
    }
};

std::vector<GameTimeTimer*> currentGameTimeTimers;
std::mutex currentGameTimeTimersMutex;

void AddToCurrentGameTimeTimers(GameTimeTimer* timer) {
    std::lock_guard<std::mutex> lock(currentGameTimeTimersMutex);
    currentGameTimeTimers.push_back(timer);
}

void EraseFinishedGameTimers() {
    std::lock_guard<std::mutex> lock(currentGameTimeTimersMutex);

    if (currentGameTimeTimers.size() == 0) {
        gameTimersEmpty = true;
        return;
    }

    erasingGameTimers = true;

    for (int i = 0; i < currentGameTimeTimers.size(); i++) {
        auto* timer = currentGameTimeTimers[i];
        if (timer) {
            if (timer->finished || timer->cancelled) {
                delete timer;
                timer = nullptr;
                //logger::trace("erased GameTimeTimers, Timers left = {}", currentGameTimeTimers.size());
            }
        }

        if (!timer) {
            auto it = currentGameTimeTimers.begin() + i;
            currentGameTimeTimers.erase(it);
            i--; //move i back 1 cause 1 timer was just erased
        }
    
    }

    if (currentGameTimeTimers.size() == 0) {
        gameTimersEmpty = true;
    }

    erasingGameTimers = false;
}

GameTimeTimer* GetTimer(std::vector<GameTimeTimer*>& v, RE::VMHandle akHandle, int aiTimerID) {
    std::lock_guard<std::mutex> lock(currentGameTimeTimersMutex);

    if (v.size() == 0) {
        return nullptr;
    }

    for (int i = 0; i < v.size(); i++) {
        auto* timer = v[i];
        if (timer) {
            if (!timer->cancelled && !timer->finished) {
                if (timer->handle == akHandle && timer->timerID == aiTimerID) {
                    return timer;
                }
            }
        }
    }

    return nullptr;
}

bool SaveTimers(std::vector<GameTimeTimer*>& v, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    std::lock_guard<std::mutex> lock(currentGameTimeTimersMutex);

    if (!a_intfc->OpenRecord(record, 1)) {
        logger::error("MenuModeTimers Failed to open record[{}]", record);
        return false;
    }

    const std::size_t size = v.size();

    if (!a_intfc->WriteRecordData(size)) {
        logger::error("record[{}] Failed to write size of MenuModeTimers!", record);
        return false;
    }

    for (auto* timer : v) {
        if (timer) {

            if (!a_intfc->WriteRecordData(timer->startTime)) {
                logger::error("record[{}] Failed to write startTime", record);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->endTime)) {
                logger::error("record[{}] Failed to write endTime", record);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->handle)) {
                logger::error("record[{}] Failed to write handle[{}]", record, timer->handle);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->timerID)) {
                logger::error("record[{}] Failed to write timerID[{}]", record, timer->timerID);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->cancelled)) {
                logger::error("record[{}] Failed to write cancelled[{}]", record, timer->cancelled);
                return false;
            }

            if (!a_intfc->WriteRecordData(timer->finished)) {
                logger::error("record[{}] Failed to write finished[{}]", record, timer->finished);
                return false;
            }

            logger::debug("gameTimer saved : startTime[{}], endTime[{}], handle[{}], ID[{}], cancelled[{}], finished[{}]",
                timer->startTime, timer->endTime, timer->handle, timer->timerID, timer->cancelled, timer->finished);
        }
        else {
            float startTime = 0.0;
            float endTime = 0.0;
            float initGameHoursInterval = 0.1;
            RE::VMHandle handle;
            int ID = -1;
            bool cancelled = true;
            bool finished = true;

            if (!a_intfc->WriteRecordData(startTime)) {
                logger::error("record[{}] Failed to write no Timer startTime", record);
                return false;
            }

            if (!a_intfc->WriteRecordData(endTime)) {
                logger::error("record[{}] Failed to write no Timer endTime", record);
                return false;
            }

            if (!a_intfc->WriteRecordData(handle)) {
                logger::error("record[{}] Failed to write no timer handle", record);
                return false;
            }

            if (!a_intfc->WriteRecordData(ID)) {
                logger::error("record[{}] Failed to write no timer Id", record);
                return false;
            }

            if (!a_intfc->WriteRecordData(cancelled)) {
                logger::error("record[{}] Failed to write no timer cancelled[{}]", record, cancelled);
                return false;
            }

            if (!a_intfc->WriteRecordData(finished)) {
                logger::error("record[{}] Failed to write no timer finished[{}]", record, finished);
                return false;
            }
        }
    }
    return true;
}

bool loadTimers(std::vector<GameTimeTimer*>& v, uint32_t record, SKSE::SerializationInterface* a_intfc) {
    std::lock_guard<std::mutex> lock(currentGameTimeTimersMutex);

    std::size_t size;
    if (!a_intfc->ReadRecordData(size)) {
        logger::error("record[{}] Failed to load size of MenuModeTimers!", record);
        return false;
    }

    if (v.size() > 0) { //cancel current timers
        for (auto* timer : v) {
            if (timer) {
                timer->cancelled = true;
            }
        }
    }

    if (!sv::calendar) {
        logger::critical("sv::calendar not found, aborting load.");
        return false;
    }

    for (std::size_t i = 0; i < size; i++) {
        float startTime = 0.0;
        float endTime = 0.0;
        float initGameHoursInterval = 0.1;
        RE::VMHandle handle;
        int ID = -1;
        bool cancelled = true;
        bool finished = true;

        if (!a_intfc->ReadRecordData(startTime)) {
            logger::error("{}: MenuModeTimer Failed to load startTime!", i);
            return false;
        }

        if (!a_intfc->ReadRecordData(endTime)) {
            logger::error("{}: MenuModeTimer Failed to load endTime!", i);
            return false;
        }

        if (!a_intfc->ReadRecordData(handle)) {
            logger::error("{}: MenuModeTimer Failed to load handle!", i);
            return false;
        }

        if (!a_intfc->ReadRecordData(ID)) {
            logger::error("{}: MenuModeTimer Failed to load ID!", i);
            return false;
        }

        if (!a_intfc->ReadRecordData(cancelled)) {
            logger::error("{}: MenuModeTimer Failed to load cancelled!", i);
            return false;
        }

        if (!a_intfc->ReadRecordData(finished)) {
            logger::error("{}: MenuModeTimer Failed to load finished!", i);
            return false;
        }

        if (!a_intfc->ResolveHandle(handle, handle)) {
            logger::warn("{}: MenuModeTimer warning, failed to resolve handle[{}]", i, handle);
            continue;
        }

        if (cancelled || finished) {
            continue;
        }

        GameTimeTimer* timer = new GameTimeTimer(handle, (endTime - sv::calendar->GetHoursPassed()), ID);
        v.push_back(timer);

        logger::debug("gameTimer loaded: startTime[{}], endTime[{}], handle[{}], ID[{}], cancelled[{}], finished[{}]",
            timer->startTime, timer->endTime, timer->handle, timer->timerID, timer->cancelled, timer->finished);
    }
    return true;
}

//timer papyrus functions==============================================================================================================================================

//forms ==============================================================================================================================================

//menuModeTimer=======================================================================================================================
void StartMenuModeTimerOnForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, float afInterval, int aiTimerID) {
    logger::trace("called");

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("eventReceiver not found");
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for form [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
        logger::debug("reset timer on form [{}] ID[{:x}] timerID[{}]", gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID(), aiTimerID);
    }

    MenuModeTimer* newTimer = new MenuModeTimer(handle, afInterval, aiTimerID);
    AddToCurrentMenuModeTimers(newTimer);
}

void CancelMenuModeTimerOnForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::trace("called, ID {}", aiTimerID);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("eventReceiver not found");
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for form [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
}

float GetTimeElapsedOnMenuModeTimerForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("eventReceiver not found");
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for form [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return -1.0;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnMenuModeTimerForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("eventReceiver not found");
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for form [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return -1.0;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//NoMenuModeTimer=======================================================================================================================
void StartNoMenuModeTimerOnForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, float afInterval, int aiTimerID) {
    logger::trace("called, time: {}", afInterval);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("eventReceiver not found");
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for form [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    NoMenuModeTimer* newTimer = new NoMenuModeTimer(handle, afInterval, aiTimerID);
    AddToCurrentNoMenuModeTimers(newTimer);
    /*std::thread tAddToCurrentNoMenuModeTimers(AddToCurrentNoMenuModeTimers, newTimer);
    tAddToCurrentNoMenuModeTimers.join();*/
}

void CancelNoMenuModeTimerOnForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("eventReceiver not found");
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for form [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnNoMenuModeTimerForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("eventReceiver not found");
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for form [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return -1.0;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnNoMenuModeTimerForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("eventReceiver not found");
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for form [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return -1.0;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//timer=======================================================================================================================
void StartTimerOnForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, float afInterval, int aiTimerID) {
    logger::trace("called, time: {}", afInterval);

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("eventReceiver not found");
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for form [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    Timer* newTimer = new Timer(handle, afInterval, aiTimerID);
    AddToCurrentTimers(newTimer);
}

void CancelTimerOnForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("eventReceiver not found");
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for form [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnTimerForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("eventReceiver not found");
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for form [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return -1.0;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnTimerForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("eventReceiver not found");
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for form [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return -1.0;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//gametime timer===========================================================================================================================
void StartGameTimerOnForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, float afInterval, int aiTimerID) {
    logger::trace("called");

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("eventReceiver not found");
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for form [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    GameTimeTimer* newTimer = new GameTimeTimer(handle, afInterval, aiTimerID);
    AddToCurrentGameTimeTimers(newTimer);
}

void CancelGameTimerOnForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("eventReceiver not found");
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for form [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnGameTimerForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("eventReceiver not found");
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for form [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return -1.0;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnGameTimerForm(RE::StaticFunctionTag*, RE::TESForm* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!gfuncs::IsFormValid(eventReceiver)) {
        logger::warn("eventReceiver not found");
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for form [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver), eventReceiver->GetFormID());
        return -1.0;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//timer Alias==============================================================================================================================================

//menu Mode Timer alias=======================================================================================================================
void StartMenuModeTimerOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, float afInterval, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for Alias [{}] ID [{:x}]", eventReceiver->aliasName, eventReceiver->aliasID);
        return;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
        logger::debug("reset timer on Alias [{}] ID[{:x}] timerID[{}]", eventReceiver->aliasName, eventReceiver->aliasID, aiTimerID);
    }

    MenuModeTimer* newTimer = new MenuModeTimer(handle, afInterval, aiTimerID);
    AddToCurrentMenuModeTimers(newTimer);
}

void CancelMenuModeTimerOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::trace("called, ID {}", aiTimerID);

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for Alias [{}] ID [{:x}]", eventReceiver->aliasName, eventReceiver->aliasID);
        return;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnMenuModeTimerAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for Alias [{}] ID [{:x}]", eventReceiver->aliasName, eventReceiver->aliasID);
        return -1.0;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnMenuModeTimerAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for Alias [{}] ID [{:x}]", eventReceiver->aliasName, eventReceiver->aliasID);
        return -1.0;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//NoMenuModeTimer Alias=======================================================================================================================
void StartNoMenuModeTimerOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, float afInterval, int aiTimerID) {
    logger::trace("called, time: {}", afInterval);

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for alias [{}] ID [{:x}]", eventReceiver->aliasName, eventReceiver->aliasID);
        return;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    NoMenuModeTimer* newTimer = new NoMenuModeTimer(handle, afInterval, aiTimerID);
    AddToCurrentNoMenuModeTimers(newTimer);
    std::thread tAddToCurrentNoMenuModeTimers(AddToCurrentNoMenuModeTimers, newTimer);
    tAddToCurrentNoMenuModeTimers.join();
}

void CancelNoMenuModeTimerOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for alias [{}] ID [{:x}]", eventReceiver->aliasName, eventReceiver->aliasID);
        return;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnNoMenuModeTimerAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for alias [{}] ID [{:x}]", eventReceiver->aliasName, eventReceiver->aliasID);
        return -1.0;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnNoMenuModeTimerAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for alias [{}] ID [{:x}]", eventReceiver->aliasName, eventReceiver->aliasID);
        return -1.0;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//timer alias=======================================================================================================================
void StartTimerOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, float afInterval, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for Alias [{}] ID [{:x}]", eventReceiver->aliasName, eventReceiver->aliasID);
        return;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    Timer* newTimer = new Timer(handle, afInterval, aiTimerID);
    AddToCurrentTimers(newTimer);
}

void CancelTimerOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for Alias [{}] ID [{:x}]", eventReceiver->aliasName, eventReceiver->aliasID);
        return;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnTimerAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for Alias [{}] ID [{:x}]", eventReceiver->aliasName, eventReceiver->aliasID);
        return -1.0;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnTimerAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for Alias [{}] ID [{:x}]", eventReceiver->aliasName, eventReceiver->aliasID);
        return -1.0;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//gametime timer alias===========================================================================================================================
void StartGameTimerOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, float afInterval, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for Alias [{}] ID [{:x}]", eventReceiver->aliasName, eventReceiver->aliasID);
        return;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    GameTimeTimer* newTimer = new GameTimeTimer(handle, afInterval, aiTimerID);
    AddToCurrentGameTimeTimers(newTimer);
}

void CancelGameTimerOnAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for Alias [{}] ID [{:x}]", eventReceiver->aliasName, eventReceiver->aliasID);
        return;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnGameTimerAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for Alias [{}] ID [{:x}]", eventReceiver->aliasName, eventReceiver->aliasID);
        return -1.0;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnGameTimerAlias(RE::StaticFunctionTag*, RE::BGSBaseAlias* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for Alias [{}] ID [{:x}]", eventReceiver->aliasName, eventReceiver->aliasID);
        return -1.0;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//timer ActiveMagicEffect==============================================================================================================================================

//menu Mode Timer ActiveMagicEffect=======================================================================================================================
void StartMenuModeTimerOnActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, float afInterval, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
        logger::debug("reset timer on ActiveMagicEffect [{}] ID[{:x}] timerID[{}]", gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID, aiTimerID);
    }

    MenuModeTimer* newTimer = new MenuModeTimer(handle, afInterval, aiTimerID);
    AddToCurrentMenuModeTimers(newTimer);
}

void CancelMenuModeTimerOnActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::trace("called, ID {}", aiTimerID);

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnMenuModeTimerActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return -1.0;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}


float GetTimeLeftOnMenuModeTimerActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return -1.0;
    }

    MenuModeTimer* timer = GetTimer(currentMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//NoMenuModeTimer ActiveMagicEffect=======================================================================================================================
void StartNoMenuModeTimerOnActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, float afInterval, int aiTimerID) {
    logger::trace("called, time: {}", afInterval);

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    NoMenuModeTimer* newTimer = new NoMenuModeTimer(handle, afInterval, aiTimerID);
    AddToCurrentNoMenuModeTimers(newTimer);
    /*std::thread tAddToCurrentNoMenuModeTimers(AddToCurrentNoMenuModeTimers, newTimer);
    tAddToCurrentNoMenuModeTimers.join();*/
}

void CancelNoMenuModeTimerOnActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnNoMenuModeTimerActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return -1.0;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnNoMenuModeTimerActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return -1.0;
    }

    NoMenuModeTimer* timer = GetTimer(currentNoMenuModeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//timer ActiveMagicEffect=======================================================================================================================
void StartTimerOnActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, float afInterval, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    Timer* newTimer = new Timer(handle, afInterval, aiTimerID);
    AddToCurrentTimers(newTimer);
}

void CancelTimerOnActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnTimerActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return -1.0;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnTimerActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return -1.0;
    }

    Timer* timer = GetTimer(currentTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

//gametime timer ActiveMagicEffect===========================================================================================================================
void StartGameTimerOnActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, float afInterval, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }

    GameTimeTimer* newTimer = new GameTimeTimer(handle, afInterval, aiTimerID);
    AddToCurrentGameTimeTimers(newTimer);
}

void CancelGameTimerOnActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        timer->cancelled = true;
    }
    return;
}

float GetTimeElapsedOnGameTimerActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return -1.0;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetElapsedTime();
        }
    }
    return  -1.0;
}

float GetTimeLeftOnGameTimerActiveMagicEffect(RE::StaticFunctionTag*, RE::ActiveEffect* eventReceiver, int aiTimerID) {
    logger::trace("called");

    if (!eventReceiver) {
        logger::warn("eventReceiver not found");
        return -1.0;
    }

    RE::VMHandle handle = gfuncs::GetHandle(eventReceiver);

    if (handle == NULL) {
        logger::error("couldn't get handle for ActiveMagicEffect [{}] ID [{:x}]", gfuncs::GetFormName(eventReceiver->GetBaseObject()), eventReceiver->usUniqueID);
        return -1.0;
    }

    GameTimeTimer* timer = GetTimer(currentGameTimeTimers, handle, aiTimerID);
    if (timer) {
        if (timer->finished) {
            return 0.0;
        }
        else if (!timer->cancelled) {
            return timer->GetTimeLeft();
        }
    }
    return  -1.0;
}

namespace timers {
    size_t GetCurrentGameTimeTimersSize() {
        return currentGameTimeTimers.size();
    }

    bool IsTimerType(std::uint32_t& type) {
        return (type == 'DBT0' || type == 'DBT1' || type == 'DBT2' || type == 'DBT3');
    }

    void LoadTimers(std::uint32_t type, SKSE::SerializationInterface* a_intfc) {
        if (type == 'DBT0') {
            loadTimers(currentMenuModeTimers, 'DBT0', a_intfc);
        }
        else if (type == 'DBT1') {
            loadTimers(currentNoMenuModeTimers, 'DBT1', a_intfc);
        }
        else if (type == 'DBT2') {
            loadTimers(currentTimers, 'DBT2', a_intfc);
        }
        else if (type == 'DBT3') {
            loadTimers(currentGameTimeTimers, 'DBT3', a_intfc);
        }
    }

    void SaveTimers(SKSE::SerializationInterface* a_intfc) {
        SaveTimers(currentMenuModeTimers, 'DBT0', a_intfc);
        SaveTimers(currentNoMenuModeTimers, 'DBT1', a_intfc);
        SaveTimers(currentTimers, 'DBT2', a_intfc);
        SaveTimers(currentGameTimeTimers, 'DBT3', a_intfc);
    }

    //called after menu close event if game not paused
    void UpdateNoMenuModeTimers(float timeElapsedWhilePaused) {
        currentNoMenuModeTimersMutex.lock();

        if (currentNoMenuModeTimers.size() == 0) {
            currentNoMenuModeTimersMutex.unlock();
            return;
        }

        for (auto* noMenuModeTimer : currentNoMenuModeTimers) {
            if (noMenuModeTimer) {
                if (!noMenuModeTimer->cancelled && !noMenuModeTimer->finished) {
                    if (noMenuModeTimer->started) {
                        //noMenuModeTimer->currentInterval += timeElapsedWhilePaused;
                        //noMenuModeTimer->totalTimePaused += timeElapsedWhilePaused;
                    }
                    else {
                        noMenuModeTimer->StartTimer();
                    }
                }
            }
        }
        currentNoMenuModeTimersMutex.unlock();
    }

    //called after menu close event if game not paused
    void UpdateTimers(float timeElapsedWhilePaused) {
        currentTimersMutex.lock();

        if (currentTimers.size() == 0) {
            currentTimersMutex.unlock();
            return;
        }

        for (auto* timer : currentTimers) {
            if (timer) {
                if (!timer->cancelled && !timer->finished) {
                    if (timer->started) {
                        //timer->currentInterval += timeElapsedWhilePaused;
                        //timer->totalTimePaused += timeElapsedWhilePaused;
                    }
                    else {
                        timer->StartTimer();
                    }
                }
            }
        }
        currentTimersMutex.unlock();
    }

    bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {

        if (!vm) {
            logger::error("vm* not found!");
            return true;
        }
        else {
            logger::info("");
        }

        //form
        vm->RegisterFunction("StartTimer", "DbFormTimer", StartTimerOnForm);
        vm->RegisterFunction("CancelTimer", "DbFormTimer", CancelTimerOnForm);
        vm->RegisterFunction("GetTimeElapsedOnTimer", "DbFormTimer", GetTimeElapsedOnTimerForm);
        vm->RegisterFunction("GetTimeLeftOnTimer", "DbFormTimer", GetTimeLeftOnTimerForm);

        vm->RegisterFunction("StartNoMenuModeTimer", "DbFormTimer", StartNoMenuModeTimerOnForm);
        vm->RegisterFunction("CancelNoMenuModeTimer", "DbFormTimer", CancelNoMenuModeTimerOnForm);
        vm->RegisterFunction("GetTimeElapsedOnNoMenuModeTimer", "DbFormTimer", GetTimeElapsedOnNoMenuModeTimerForm);
        vm->RegisterFunction("GetTimeLeftOnNoMenuModeTimer", "DbFormTimer", GetTimeLeftOnNoMenuModeTimerForm);

        vm->RegisterFunction("StartMenuModeTimer", "DbFormTimer", StartMenuModeTimerOnForm);
        vm->RegisterFunction("CancelMenuModeTimer", "DbFormTimer", CancelMenuModeTimerOnForm);
        vm->RegisterFunction("GetTimeElapsedOnMenuModeTimer", "DbFormTimer", GetTimeElapsedOnMenuModeTimerForm);
        vm->RegisterFunction("GetTimeLeftOnMenuModeTimer", "DbFormTimer", GetTimeLeftOnMenuModeTimerForm);

        vm->RegisterFunction("StartGameTimer", "DbFormTimer", StartGameTimerOnForm);
        vm->RegisterFunction("CancelGameTimer", "DbFormTimer", CancelGameTimerOnForm);
        vm->RegisterFunction("GetTimeElapsedOnGameTimer", "DbFormTimer", GetTimeElapsedOnGameTimerForm);
        vm->RegisterFunction("GetTimeLeftOnGameTimer", "DbFormTimer", GetTimeLeftOnGameTimerForm);

        //Alias
        vm->RegisterFunction("StartTimer", "DbAliasTimer", StartTimerOnAlias);
        vm->RegisterFunction("CancelTimer", "DbAliasTimer", CancelTimerOnAlias);
        vm->RegisterFunction("GetTimeElapsedOnTimer", "DbAliasTimer", GetTimeElapsedOnTimerAlias);
        vm->RegisterFunction("GetTimeLeftOnTimer", "DbAliasTimer", GetTimeLeftOnTimerAlias);

        vm->RegisterFunction("StartNoMenuModeTimer", "DbAliasTimer", StartNoMenuModeTimerOnAlias);
        vm->RegisterFunction("CancelNoMenuModeTimer", "DbAliasTimer", CancelNoMenuModeTimerOnAlias);
        vm->RegisterFunction("GetTimeElapsedOnNoMenuModeTimer", "DbAliasTimer", GetTimeElapsedOnNoMenuModeTimerAlias);
        vm->RegisterFunction("GetTimeLeftOnNoMenuModeTimer", "DbAliasTimer", GetTimeLeftOnNoMenuModeTimerAlias);

        vm->RegisterFunction("StartMenuModeTimer", "DbAliasTimer", StartMenuModeTimerOnAlias);
        vm->RegisterFunction("CancelMenuModeTimer", "DbAliasTimer", CancelMenuModeTimerOnAlias);
        vm->RegisterFunction("GetTimeElapsedOnMenuModeTimer", "DbAliasTimer", GetTimeElapsedOnMenuModeTimerAlias);
        vm->RegisterFunction("GetTimeLeftOnMenuModeTimer", "DbAliasTimer", GetTimeLeftOnMenuModeTimerAlias);

        vm->RegisterFunction("StartGameTimer", "DbAliasTimer", StartGameTimerOnAlias);
        vm->RegisterFunction("CancelGameTimer", "DbAliasTimer", CancelGameTimerOnAlias);
        vm->RegisterFunction("GetTimeElapsedOnGameTimer", "DbAliasTimer", GetTimeElapsedOnGameTimerAlias);
        vm->RegisterFunction("GetTimeLeftOnGameTimer", "DbAliasTimer", GetTimeLeftOnGameTimerAlias);

        //ActiveMagicEffect
        vm->RegisterFunction("StartTimer", "DbActiveMagicEffectTimer", StartTimerOnActiveMagicEffect);
        vm->RegisterFunction("CancelTimer", "DbActiveMagicEffectTimer", CancelTimerOnActiveMagicEffect);
        vm->RegisterFunction("GetTimeElapsedOnTimer", "DbActiveMagicEffectTimer", GetTimeElapsedOnTimerActiveMagicEffect);
        vm->RegisterFunction("GetTimeLeftOnTimer", "DbActiveMagicEffectTimer", GetTimeLeftOnTimerActiveMagicEffect);

        vm->RegisterFunction("StartNoMenuModeTimer", "DbActiveMagicEffectTimer", StartNoMenuModeTimerOnActiveMagicEffect);
        vm->RegisterFunction("CancelNoMenuModeTimer", "DbActiveMagicEffectTimer", CancelNoMenuModeTimerOnActiveMagicEffect);
        vm->RegisterFunction("GetTimeElapsedOnNoMenuModeTimer", "DbActiveMagicEffectTimer", GetTimeElapsedOnNoMenuModeTimerActiveMagicEffect);
        vm->RegisterFunction("GetTimeLeftOnNoMenuModeTimer", "DbActiveMagicEffectTimer", GetTimeLeftOnNoMenuModeTimerActiveMagicEffect);

        vm->RegisterFunction("StartMenuModeTimer", "DbActiveMagicEffectTimer", StartMenuModeTimerOnActiveMagicEffect);
        vm->RegisterFunction("CancelMenuModeTimer", "DbActiveMagicEffectTimer", CancelMenuModeTimerOnActiveMagicEffect);
        vm->RegisterFunction("GetTimeElapsedOnMenuModeTimer", "DbActiveMagicEffectTimer", GetTimeElapsedOnMenuModeTimerActiveMagicEffect);
        vm->RegisterFunction("GetTimeLeftOnMenuModeTimer", "DbActiveMagicEffectTimer", GetTimeLeftOnMenuModeTimerActiveMagicEffect);

        vm->RegisterFunction("StartGameTimer", "DbActiveMagicEffectTimer", StartGameTimerOnActiveMagicEffect);
        vm->RegisterFunction("CancelGameTimer", "DbActiveMagicEffectTimer", CancelGameTimerOnActiveMagicEffect);
        vm->RegisterFunction("GetTimeElapsedOnGameTimer", "DbActiveMagicEffectTimer", GetTimeElapsedOnGameTimerActiveMagicEffect);
        vm->RegisterFunction("GetTimeLeftOnGameTimer", "DbActiveMagicEffectTimer", GetTimeLeftOnGameTimerActiveMagicEffect);

        return true;
    }
}
