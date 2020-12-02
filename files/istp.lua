function teleported(fx, fy, tx, ty, prtl)
    GlobalsSetValue('justTeleported', '30')
    GamePrint(fx .. ', ' .. fy .. ', ' .. tx .. ', ' .. ty .. ', ' .. tostring(prtl) .. ', ')
end
