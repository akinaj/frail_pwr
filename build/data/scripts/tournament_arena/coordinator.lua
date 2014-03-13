 -- Script for coordinator. Author: Sebastian Łasisz

respawn_time = 60

explosion_time = {0, 0, 0 ,0, 0, 0}
objects = {}
objects_name = {"eo_b1", "eo_b2", "eo_b3", "eo_b4", "cvt_hp", "cvt_dd"}
objects_type = {"barrel","barrel","barrel","barrel", "hp", "dd"}
objects_position = {}
size = 0
done = { }

dd_timer = 0
done_time_dd = false
dont_t = true

function onPostCreate()
	size = table.getn(objects_name) + 1
	createReferences(size)
	createObjectsPosition(size)
end

function onUpdate()
	createReferences(size)
	createObjects(size)
	dd_time()
	dd_controller()
end
 
function createReferences(k)
	i = 1
	repeat
		objects[i] = FindObject(objects_name[i])
		i = i+1
	until i == k
end 

function createObjectsPosition(k)
	i = 1
	repeat
		objects_position[i] = GetWorldPosition(FindObject(objects_name[i]):GetObject())
		i = i + 1
	until i == k
end

function createObjects(k)
	i = 1
	repeat 
		createObject(i)
		i = i + 1
	until i == k
end

function createObject(i)
	if not objects[i]:IsValid()  then 
		if done[i] then 
			explosion_time[i] = GetTime()
			done[i] = false
		end
		if (explosion_time[i]+respawn_time)<GetTime() and not done[i] then
			if(objects_type[i] == "barrel") then
				barrel = CreateObject(this, "ExplodingObject", "barrel_e_r8")
				SetFieldValue(barrel:GetObject(), "Name", objects_name[i])
				SetWorldPosition(barrel:GetObject(), objects_position[i])
				objects[i] = FindObject(objects_name[i])
				done[i] = true
			end
			if(objects_type[i] == "hp") then
				hp = CreateObject(this, "CylinderVolumeTrigger", "healing_bandage")
				SetFieldValue(hp:GetObject(), "Name", "cvt_hp")
				SetWorldPosition(hp:GetObject(), mkVec3.new(-22.0, 0.0, 0.0))
				objects[i] =  FindObject("cvt_hp")
				done[i] = true
			end
			if(objects_type[i] == "dd") then
				dd = CreateObject(this, "CylinderVolumeTrigger", "double_damage")
				SetFieldValue(dd:GetObject(), "Name", "cvt_dd")
				SetWorldPosition(dd:GetObject(), mkVec3.new(0.0, 0.0, 0.0))
				objects[i] =  FindObject("cvt_dd")
				done[i] = true
			end
		end
	end	
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
		objects_in_radius = FindObjectsInRadius(pos, 5)
		for i,v in ipairs(objects_in_radius) do
			if IsDerivedOrExactClass(v:GetObject(), "Character") then
				pos_act = GetWorldPosition(v:GetObject())
				SetWorldPosition(bilboard:GetObject(), (pos_act + mkVec3.new(0.0, 2.1, 0.0)))
				if (dd_timer + 5 < GetTime()) and dont_t then
					dont_t = false
					SetFieldValue(v:GetObject(), "m_shootingDamage", GetFieldValue(v:GetObject(), "m_shootingDamage") / 2)	
					DestroyObject(bilboard:GetObject())						
				end
			end
		end
	else
		dont_t = true
		done_time_dd = false
	end
 end