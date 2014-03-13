-- Script for claymore trap. Author: Sebastian Łasisz

DMG_FIRE = 2
dmg_value = 15

function onTouched(touched_object)
	if IsDerivedOrExactClass(touched_object:GetObject(), "Player") then
		GiveDamage(this, touched_object, DMG_FIRE, dmg_value, mkVec3.new(), mkVec3.new())		
		GameObject.DestroyObject(this)				
	end
end

function onUpdate()
	if (math.floor(GetTime()+1) % 33 == 0) then
		create = false
		GameObject.DestroyObject(this)
	end
end