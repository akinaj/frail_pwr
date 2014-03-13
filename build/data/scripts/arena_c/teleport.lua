-- Script for teleport. Author: Sebastian Łasisz

done = true

function onTouched(touched_object)
	if IsDerivedOrExactClass(touched_object:GetObject(), "Character") and not done then
		if GetObjectName(this) == "cvt_teleport" then
			Character.TeleportTo(touched_object:GetObject(), mkVec3.new(-48.0, 34.0, -48.0))
		end
	end
end

function onUpdate()
		crystal = FindObject("cvt_key")
		if not crystal:IsValid() and done then
			cloud = CreateObject(this, "ModelObject", "teleport_2")
			SetFieldValue(cloud:GetObject(), "Name", "cvt_teleport_1")
			SetWorldPosition(cloud:GetObject(), GetWorldPosition(this))
			done = false
		end
end