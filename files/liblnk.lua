-- Copyright (c) 2020 Reese Danzer

-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

local ffi = ffi or _G.ffi or require("ffi")

--local status, clib = pcall(ffi.load, "mods/advanced_cartography/files/capture/capture_mt")
local status, clib = pcall(ffi.load, "mods/advanced_cartography/files/cap/capture_mt")
if not status then
	print("Error loading capture lib: " .. clib)
end
ffi.cdef [[
    bool capture(const char*, const int, const int);
]]

function captureWrapper(path, vResX, vResY)
    return(clib.capture(path, vResX, vResY))
end
