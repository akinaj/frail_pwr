-- Script for teleport. Author: Sebastian Łasisz

done = true
teleport_time = 0
waiting_time = 20

function onTouched(touched_object)
	if IsDerivedOrExactClass(touched_object:GetObject(), "Character") and done then
		if GetObjectName(this) == "teleport_1" then
			Character.TeleportTo(touched_object:GetObject(), mkVec3.new(-34.0, 2.0, -35.0))
			cloud = FindObject("cvt_teleport_1_1")
			GameObject.DestroyObject(cloud:GetObject())
		end
		if GetObjectName(this) == "teleport_2" then
			Character.TeleportTo(touched_object:GetObject(), mkVec3.new(35.0, 2.0, 34.0))
			cloud = FindObject("cvt_teleport_2_1")
			GameObject.DestroyObject(cloud:GetObject())
		end	
		
		if done then 
			teleport_time = GetTime()
			done = false
		end
	end
end

function onUpdate()
	if not done and (waiting_time + teleport_time) < GetTime() then
		done = true
		if GetObjectName(this) == "teleport_1" then
			cloud = CreateObject(this, "ModelObject", "teleport_1")
			SetFieldValue(cloud:GetObject(), "Name", "cvt_teleport_1_1")
			SetWorldPosition(cloud:GetObject(), GetWorldPosition(this))
		end
		if GetObjectName(this) == "teleport_2" then
			cloud = CreateObject(this, "ModelObject", "teleport_1")
			SetFieldValue(cloud:GetObject(), "Name", "cvt_teleport_2_1")
			SetWorldPosition(cloud:GetObject(), GetWorldPosition(this))
		end			
	end
end