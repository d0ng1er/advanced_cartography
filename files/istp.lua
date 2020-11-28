require('os')
--This doesn't work, can't require OS after game start.
--Need to either initialize it beforehand or find a better way.

function sleep(n)
  if n > 0 then os.execute("ping -n " .. tonumber(n+1) .. " localhost > NUL") end
end


function teleported(from_x, from_y, to_x, to_y, portal_teleport)
    _G.isTeleporting = true
    start = GameGetFrameNum()
    GamePrint('isTeleporting, frame: ' .. tostring(start))
	sleep(2)
    _G.isTeleporting = false
    GamePrint('isTeleporting false')
end

