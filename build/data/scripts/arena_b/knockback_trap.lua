-- Script for knockback trap. Author: Sebastian Łasisz

create = true

function onTouched(touched_object)
	if IsDerivedOrExactClass(touched_object:GetObject(), "Player") then
		pos = GetWorldPosition(this) + mkVec3.new(0.0, 0.0, 4.0)
		Character.TeleportTo(touched_object:GetObject(), pos)
		GameObject.DestroyObject(this)
	end
end

function onDestroy()
	if create then 
		pos = GetWorldPosition(this) + mkVec3.new(0.0, -8.0, 4.0)
		trap = CreateObject(this, "CylinderVolumeTrigger", "land_mine_r2")
		SetFieldValue(trap:GetObject(), "Name", "land_mine_r2")
		SetWorldPosition(trap:GetObject(), pos)
	end
end

function onUpdate()
	ModelObject.SetOrientation(this, mkQuat.new(mkVec3.new(0, 1, 0), 270))
	if (math.floor(GetTime()+1) % 30	 == 0) then
		create = false
		GameObject.DestroyObject(this)
	end
end