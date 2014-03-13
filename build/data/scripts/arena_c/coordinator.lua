 -- Script for coordinator. Author: Sebastian Łasisz

respawn_time = 30
explosion_time = {0, 0, 0 ,0, 0}
barrels = {}
done_speed = false
barrels_name = {"eo_b1", "eo_b2", "eo_b3", "eo_b4", "eo_b5"}
barrels_position = {}
hp2_spawned = false
hp_spawned = false
size = 0
done = { }
done_time = {}
barrels_time =  {0, 0, 0, 0, 0}

dd_timer = 0
done_time_dd = false
dont_t = true

function onPostCreate()
	size = table.getn(barrels_name) 
	createReferances(size)
	createBarrelsPosition(size)
	setDone(size)
	setDoneTime(size)
end


function onUpdate()
	createReferances(size)
	createBarrels(size)
	dd_time()	
	dd_controller()	
	createDD()
end


function createDD()
	dd = FindObject("cvt_dd")
	if not dd:IsValid() and (math.floor(GetTime()%62)==0) then
 		dd = CreateObject(this, "CylinderVolumeTrigger", "double_damage")
		SetFieldValue(dd:GetObject(), "Name", "cvt_dd")
		SetWorldPosition(dd:GetObject(), mkVec3.new( -20.0, 0.0, -44.0 ))
	end
end
function getBarrelTime()
	a = 1
	repeat
		if (not barrels[a]:IsValid()) and done_time[a] then
			barrels_time[a] = GetTime()
			done_time[a] = false
		end
		a = a +1;
	until a == size
end

function createBarrelsPosition(k)
	i = 1
	repeat
		barrels_position[i] = GetWorldPosition(FindObject(barrels_name[i]):GetObject())
		i = i + 1
	until i == k
end

function setDone(k)
	i = 1
	repeat
		done[i] = true
		i = i + 1
	until i == k
end

function setDoneTime(k)
	i = 1
	repeat
		done_time[i] = true
		i = i + 1
	until i == k
end

function createBarrels(k)
	i = 1
	repeat 
		createBarrel(i)
		i = i + 1
	until i == k
end

function createBarrel(i)
	if not barrels[i]:IsValid()  then 
		if done[i] then 
			explosion_time[i] = GetTime()
			done[i] = false
		end
		if (explosion_time[i]+respawn_time)<GetTime() and not done[i] then
				barrel = CreateObject(this, "ExplodingObject", "barrel_e_r4")
				SetFieldValue(barrel:GetObject(), "Name", barrels_name[i])
				SetWorldPosition(barrel:GetObject(), barrels_position[i])
				barrels[i] = FindObject(barrels_name[i])
				done[i] = true
		end
	end	
end


function createReferances(k)
	i = 1
	repeat
		barrels[i] = FindObject(barrels_name[i])
		i = i+1
	until i == k
end


 function dd_time()
	dd = FindObject("cvt_dd")
	if dd:IsValid() then 
		done_time_dd = false
	end
	if not dd:IsValid() and not done_time_dd then
		dd_timer = GetTime()
		done_time_dd = true
	end
 end
 
 function dd_controller()
	dd = FindObject("cvt_dd")
	bilboard = FindObject("bilbcvt_dd")	
	if bilboard:IsValid() and done_time_dd then
		pos = GetWorldPosition(bilboard:GetObject())
		objects_in_radius = FindObjectsInRadius(pos, 8)
		for i,v in ipairs(objects_in_radius) do
			if IsDerivedOrExactClass(v:GetObject(), "Character") then
				pos_act = GetWorldPosition(v:GetObject())
				SetWorldPosition(bilboard:GetObject(), (pos_act + mkVec3.new(0.0, 2.1, 0.0)))
				if (dd_timer + 10 < GetTime()) and dont_t then
					dont_t = false
					SetFieldValue(v:GetObject(), "m_shootingDamage", GetFieldValue(v:GetObject(), "m_shootingDamage") / 2)	
					DestroyObject(bilboard:GetObject())						
				end
			end
		end
	else
		dont_t = true
	end
 end