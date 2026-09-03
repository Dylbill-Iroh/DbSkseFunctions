#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#include <format>
#include <string_view>

// ---------------------------------------------------------------------------
// std::formatter for RE::BSFixedString
//
// CommonLibSSE-NG ships a fmt::formatter for BSFixedString, which is what the
// old vcpkg/CMake build used. This xmake template builds spdlog with
// SPDLOG_USE_STD_FORMAT, so logging goes through std::format instead -- and
// std::format has no formatter for BSFixedString, so every
//     logger::trace("...{}...", gfuncs::GetFormName(form))
// fails with "error C2039: 'parse': is not a member of std::formatter<...>".
//
// GetFormName() returns BSFixedString and is logged in ~233 places, so
// specialize once here instead of touching every call site.
//
// If a future CommonLibSSE-NG adds its own std::formatter for BSFixedString
// this will start failing as a duplicate specialization -- delete this block
// if that happens.
// ---------------------------------------------------------------------------
template <>
struct std::formatter<RE::BSFixedString, char> : std::formatter<std::string_view, char>
{
    template <class FormatContext>
    auto format(const RE::BSFixedString& a_str, FormatContext& a_ctx) const
    {
        const char* p = a_str.c_str();
        return std::formatter<std::string_view, char>::format(
            p ? std::string_view{ p } : std::string_view{}, a_ctx);
    }
};

// ---------------------------------------------------------------------------
// Safety net for Windows ANSI/Unicode macro collisions.
//
// WIN32_LEAN_AND_MEAN (set in xmake.lua) keeps winspool.h and mmsystem.h out of
// windows.h, which is the real fix. These #undefs cover anything that slips
// through anyway -- notably wingdi.h's GetObject, which WIN32_LEAN_AND_MEAN
// does NOT exclude (that one needs NOGDI, which is riskier to define).
//
// Must come AFTER the RE/SKSE includes above, since that is what drags in
// windows.h.
// ---------------------------------------------------------------------------
#undef AddForm
#undef DeleteForm
#undef GetForm
#undef SetForm
#undef EnumForms
#undef PlaySound
#undef GetObject
#undef LoadString

using namespace std::literals;
namespace logger = SKSE::log;