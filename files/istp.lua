-- istp detects player teleports and sets a frame counter which is incremented in init.
-- if TP is from a portal, the most recent capture will also be deleted (also from init)
-- this is because the portal tps and TPs in general move the camera in very strange ways

function teleported(fx, fy, tx, ty, prtl)
    if prtl then
        GlobalsSetValue('justTeleported', '30')
    else
        GlobalsSetValue('justTeleported', '15')
    end
    --GamePrint(fx .. ', ' .. fy .. ', ' .. tx .. ', ' .. ty .. ', ' .. tostring(prtl) .. ', ')
end
