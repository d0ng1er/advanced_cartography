function teleported(fx, fy, tx, ty, prtl)
    if prtl then
        GlobalsSetValue('justTeleported', '30')
    else
        GlobalsSetValue('justTeleported', '15')
    end
    --GamePrint(fx .. ', ' .. fy .. ', ' .. tx .. ', ' .. ty .. ', ' .. tostring(prtl) .. ', ')
end
