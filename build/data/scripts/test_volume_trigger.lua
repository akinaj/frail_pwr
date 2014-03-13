-- Test script that implements volume trigger as scripted ModelObject
-- For now, just test sphere volume around trigger

test_radius = 10.0
done = false

last_update_time = -1.0
check_interval = 0.5

player = nil

function onPostCreate()
    -- Just for testing, there is also much faster function GetPlayer
    player = FindObject("Player")
end

function onUpdate()
	if (done) then
		return
	end
    
    -- Update every check_interval seconds
    if (last_update_time >= 0 and GetTime() - last_update_time < check_interval) then
        return
    end
    
    last_update_time = GetTime()

    if not player:IsValid() then
        LogError("i haz no player :(")
    end
    
	player_world_pos = GetWorldPosition(player:GetObject())
	my_world_pos = GetWorldPosition(this)
	dist_sq_to_centre = mkVec3.dist_sq(my_world_pos, player_world_pos)
	
	if (dist_sq_to_centre < test_radius * test_radius) then
		LogInfo("Triggering action!")
		CallScriptMethod(CreateObject(this, "ModelObject", "test_from_script"):GetObject(), "lolTest")
		done = true
	end
end