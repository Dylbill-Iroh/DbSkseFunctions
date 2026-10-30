#pragma once

namespace gfx {
	bool BindPapyrusFunctions(RE::BSScript::IVirtualMachine* vm);
    std::string GetGfxValueAsString(RE::GFxValue& gfxValue);
	bool SetArrayElementsFromString(const std::string& s, RE::GFxValue& GFxArray, RE::GPtr<RE::GFxMovieView> mv, bool setGFx); //setGFx false for validate only, if true actually sets the GFxArray values
	bool ParseUiArg(const std::string& str, RE::GFxValue& out, RE::GPtr<RE::GFxMovieView> mv, bool setGFx); //setGFx false for validate only, if true actually sets the out value
}