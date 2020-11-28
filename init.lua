dofile_once('mods/advanced_cartography/files/liblnk.lua')
dofile_once('mods/advanced_cartography/files/fm.lua')


function OnPlayerSpawned(player_entity)
    playerID = player_entity
    oldCamX, oldCamY = 0, 0
    oldCamX, oldCamY = GameGetCameraPos()
    _G.isTeleporting = false
    photoPath = pathMaster(StatsGetValue('world_seed'))
    cnt = 0
    dead = false
    ngpcnt = tostring(SessionNumbersGetValue('NEW_GAME_PLUS_COUNT'))
    -- Teleport Detection not yet working, limited heavily
    -- by game disabling mod api post initialization.
    --EntityAddComponent(
    --    playerID,
    --    'LuaComponent',
    --    {script_teleported = 'mods/advanced_cartography/scripts/istp.lua'}
    --    )
    -- death detection not working either, for the same reasons
    -- may try to fix eventually, but not right now.
    --EntityAddComponent(
    --    playerID,
    --    'LuaComponent',
    --    {script_death = 'mods/advanced_cartography/files/death.lua'}
    --    )
end


function takeSShot(vResX, vResY, newCamX, newCamY)
    local rCamX = string.format("%.2f", newCamX)
    local rCamY = string.format("%.2f", newCamY)
    local fName = cnt .. '_(' .. rCamX .. ',' .. rCamY .. ')' .. '.jpg'
    local fullPath = photoPath .. fName
    if captureWrapper(fullPath, vResX, vResY) then
        print('AC: took screenshot ' .. fName .. '\n')
        cnt = cnt + 1
    else
        print('AC: screenshot failed\n')
    end
end


function OnWorldPostUpdate() 
    local ngp = tostring(SessionNumbersGetValue('NEW_GAME_PLUS_COUNT'))
    if GameIsInventoryOpen()
       or not oldCamY
       or not ngp
       or dead
       then return
    elseif _G.isTeleporting then
        GamePrint('SS Disabled: Teleporting')
        return
    elseif ngp ~= ngpcnt then
        ngpcnt = ngp
        photoPath = pathMaster(StatsGetValue('world_seed'))
    end

    local newCamX, newCamY =GameGetCameraPos()
    local vResX = tonumber(MagicNumbersGetValue('VIRTUAL_RESOLUTION_X'))
    local vResY = tonumber(MagicNumbersGetValue('VIRTUAL_RESOLUTION_Y'))

    if (((vResX*0.25) < math.abs(oldCamX - newCamX))
        or ((vResY*0.25) < math.abs(oldCamY - newCamY)))
        then
            oldCamX, oldCamY = newCamX, newCamY
            takeSShot(vResX, vResY, newCamX, newCamY)
    elseif not EntityGetIsAlive(playerID)
        and not dead
        then
            dead = true
            oldCamX, oldCamY = newCamX, newCamY
            takeSShot(vResX, vResY, newCamX, newCamY)
            death()
    end
end
