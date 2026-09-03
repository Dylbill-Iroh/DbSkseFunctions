
-- Add CTRE and Xbyak package dependencies from xmake-repo
add_requires("ctre")
add_requires("xbyak")

-- include subprojects
includes("lib/commonlibsse-ng")

-- set project constants
set_project("doticu_skypal")
set_version("0.0.1")
set_license("GPL-3.0")
set_languages("c++23")
set_warnings("all")

-- add common rules
add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

-- define targets
target("doticu_skypal")
    add_rules("commonlibsse-ng.plugin", {
        name = "doticu_skypal",
        author = "Dylbill",
        description = "Rebuilt the Skypal Papyrus Resource using CommonLib NG."
    })

    -- Link the managed CTRE and Xbyak packages into this target
    add_packages("ctre", "xbyak")

    -- Windows SDK headers define ANSI/Unicode macro pairs that collide with
    -- this project's function names:
    --   winspool.h : AddForm -> AddFormA   (also DeleteForm/GetForm/SetForm/EnumForms)
    --   mmsystem.h : PlaySound -> PlaySoundA
    -- windows.h only pulls those headers in when WIN32_LEAN_AND_MEAN is absent.
    -- The old CMake/vcpkg build got these defines from the CommonLibSSE target's
    -- usage requirements; xmake does not set them, so set them here.
    -- add_defines("WIN32_LEAN_AND_MEAN", "NOMINMAX")

    -- 1. Source files: Track root plugin.cpp AND any fallback src files
    add_files("plugin.cpp")
    -- add_files("src/**.cpp")
    
    -- 2. Include paths and header monitoring
    add_headerfiles("PCH.h")
    -- add_headerfiles("src/**.h")
    add_headerfiles("include/**.h")
    
    -- Add root folder, src, and include to the compiler scan path
    add_includedirs(".", "src", "include")

    -- 3. Precompiled Header updated to root path location
    set_pcxxheader("PCH.h")

    -- Custom deployment step: Copies a config folder to your output directory if it exists
    after_build(function (target)
        import("core.base.option")
        local src_config_folder = path.join(os.projectdir(), "config")
        
        -- Check if you have a local config folder to deploy
        if os.isdir(src_config_folder) then
            local targetdir = target:targetdir()
            local dll_folder = path.join(targetdir, "SKSE", "Plugins")
            local dest_config_folder = path.join(dll_folder, "Template_Plugin")
            
            os.mkdir(dest_config_folder)
            os.copydir(src_config_folder, dest_config_folder)
            cprint("${green}[deploy]: Config directory copied to output successfully!")
        end
    end)