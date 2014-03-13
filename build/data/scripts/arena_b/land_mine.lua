-- Script for land mine. Author: Sebastian Łasisz

DMG_FIRE = 2
dmg_value = 5

speed = 0
done = false
time_of_explosion = 0
stun_time = 2

function onTouched(touched_object)
	if IsDerivedOrExactClass(touched_object:GetObject(), "Player") and not done then
		speed = GetFieldValue(touched_object:GetObject(), "m_maxSpeed")
		SetFieldValue(touched_object:GetObject(), "m_maxSpeed", 0.0)
		GiveDamage(this, touched_object, DMG_FIRE, dmg_value, mkVec3.new(), mkVec3.new())		
		time_of_explosion = GetTime()
		done = true
	end
	if IsDerivedOrExactClass(touched_object:GetObject(), "Player")  then
		if (time_of_explosion + stun_time) < GetTime() then
			SetFieldValue(touched_object:GetObject(), "m_maxSpeed", speed)		
			GameObject.DestroyObject(this)
		end
	end
end