dofile_once('mods/advanced_cartography/files/liblnk.lua')
dofile_once('mods/advanced_cartography/files/fm.lua')


function OnPlayerSpawned(player_entity)
    playerID = player_entity
    vResX = tonumber(MagicNumbersGetValue('VIRTUAL_RESOLUTION_X'))
    vResY = tonumber(MagicNumbersGetValue('VIRTUAL_RESOLUTION_Y'))
    oldCamX, oldCamY = 0, 0
    oldCamX, oldCamY = GameGetCameraPos()
    GlobalsSetValue('deadzone', tostring(ModSettingGet('advanced_cartography.deadzone')))
    photoPath = pathMaster(StatsGetValue('world_seed'))
    cnt = 0
    dead = false
    ngpcnt = tostring(SessionNumbersGetValue('NEW_GAME_PLUS_COUNT'))
    -- Teleport Detection not yet working, limited heavily
    -- by game disabling mod api post initialization.
    EntityAddComponent(
        playerID,
        'LuaComponent',
        {script_teleported = 'mods/advanced_cartography/files/istp.lua'}
        )
end


function takeSShot(newCamX, newCamY)
    local rCamX = string.format("%.2f", newCamX)
    local rCamY = string.format("%.2f", newCamY)
    local fName = cnt .. '_(' .. rCamX .. ',' .. rCamY .. ')' .. '.jpg'
    fullPath = photoPath .. fName
    if captureWrapper(fullPath, vResX, vResY) then
        print('AC: took screenshot ' .. fName)
        cnt = cnt + 1
    else
        print('AC: screenshot failed')
    end
end


function OnWorldPostUpdate() 
    local ngp = tostring(SessionNumbersGetValue('NEW_GAME_PLUS_COUNT'))
    if GameIsInventoryOpen()
       or not oldCamY
       or not ngp
       or dead
       then return
    elseif ngp ~= ngpcnt then
        ngpcnt = ngp
        photoPath = pathMaster(StatsGetValue('world_seed'))
    elseif GlobalsGetValue('justTeleported', '0') ~= '0' then
        num = tonumber(GlobalsGetValue('justTeleported', '0'))
        if num == 30 then
            delete(fullPath)
            print('AC: teleport detected, capture disabled, most recent screenshot deleted\n')
        end
        print('AC: ' .. num .. ' more frames until capture rearm')
        num = num - 1
        GlobalsSetValue('justTeleported', tostring(num))
        return
    end

    local newCamX, newCamY = GameGetCameraPos()
    local deadzone = tonumber(GlobalsGetValue('deadzone', '0.25'))

    if (((vResX*deadzone) < math.abs(oldCamX - newCamX))
        or ((vResY*deadzone) < math.abs(oldCamY - newCamY)))
        then
            oldCamX, oldCamY = newCamX, newCamY
            takeSShot(newCamX, newCamY)
    elseif StatsGetValue("dead") == "1"
        and not dead
        then
            dead = true
            oldCamX, oldCamY = newCamX, newCamY
            takeSShot(newCamX, newCamY)
            death()
    end
end
