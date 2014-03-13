-- Script for healing bandage. Author: Sebastian Łasisz

heal_value = 20
angle = 1

function onTouched(touched_object)	
	if IsDerivedOrExactClass(touched_object:GetObject(), "Character") then
		Character.AddHealth(touched_object:GetObject(), heal_value)
		GameObject.DestroyObject(this)
	end		
end

function onUpdate()
	angle = angle + 150 * GetTimeDelta()
	ModelObject.SetOrientation(this, mkQuat.new(mkVec3.new(0, 1, 0), angle))
end
