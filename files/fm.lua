-- Copyright (c) 2020 Reese Danzer

-- This software is released under the MIT License.
-- https://opensource.org/licenses/MIT

-- This module supplys the file-management functions to init.lua, specifically creation of output paths, map paths, and tagging of paths, as well as retagging on death

require('os')

--SOURCE
--https://stackoverflow.com/questions/1340230/check-if-directory-exists-in-lua
--check if a file or directory exists in this path
function exists(file)
    local ok, err, code = os.rename(file, file)
    if not ok then
      if code == 13 then
         -- Permission denied, but it exists
         return true
      end
    end
    return ok
end


function pathMaster(seedstr)
    --takes seedstr, makes either new seed dir or new batch dir within seed dir
    ngp = SessionNumbersGetValue('NEW_GAME_PLUS_COUNT')
    ngstr = ''
    if ngp ~= '0' then
        ngstr = '[NG' .. ngp .. ']_'
    end

    drstr = ''
    if GameIsDailyRun() then
        drstr = '[DR]_'
    end

    dprstr = ''
    if ModIsEnabled('daily_practice') then
        dprstr = '[DPR]_'
    end

    ntmrstr = ''
    if ModIsEnabled('nightmare') then
        ntmrstr = '[NTMR]_'
    end

    mapName = ngstr .. drstr .. dprstr .. ntmrstr .. seedstr

	rawPath = 'mods\\advanced_cartography\\raws\\' .. mapName .. '\\'
	if not exists(rawPath) then
        os.execute('mkdir ' .. rawPath)
    end
	mapPath = 'mods\\advanced_cartography\\maps\\' .. mapName .. '\\'
	if not exists(mapPath) then
        os.execute('mkdir ' .. mapPath)
    end
    local cnt = 0
    local photoPath = ''
    repeat
        photoPath = rawPath .. cnt .. '\\'
        cnt = cnt + 1
    until not exists(photoPath)
    os.execute ('mkdir ' .. photoPath)
    if not exists(photoPath) then
        print('AC Error: file path not initialized\n')
    end
    print('AC: Screenshot Directory: ' .. photoPath)
    return photoPath
end

function death()
    --when the player dies, this function gets called
    --and marks the seed dir with [DEAD]
    if exists(rawPath) then
        local deadPath = '[DEAD]_' .. mapName
        if exists('mods\\advanced_cartography\\raws\\' .. deadPath) then
            local cnt = 2
            repeat
                deadPath = '[DEAD' .. cnt .. 'x]_' .. mapName
                cnt = cnt + 1
            until not exists('mods\\advanced_cartography\\raws\\' .. deadPath)
        end
        print('AC: Death: map name changed to ' .. deadPath)
        os.execute('rename ' .. rawPath .. ' ' .. deadPath)
        os.execute('rename ' .. mapPath .. ' ' .. deadPath)
    else
        print("AC Error: you appear to have died in a world that doesn't exist")
    end
end
