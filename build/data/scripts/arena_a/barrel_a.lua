-- Script for barrel . Author: Sebastian Łasisz

RIBBON = nil

DMG_BULLET = 1
DMG_FIRE = 2
DMG_POISON = 3
DMG_PUNCH = 4
DMG_COLLISION = 5

explosion_delay = 0.25
explosion_time = 0
damage_mul = 1.0
explode = false


function onTakeDamage(dmg_type, dmg_value)
	if dmg_type == DMG_FIRE then
		damage_mul = 2.0
	end
	if dmg_type == DMG_FIRE or dmg_type == DMG_BULLET or dmg_type == DMG_PUNCH then
		explosion_time = GetTime()
		explode = true
	end
end

function onPostCreate()
	pos = GetWorldPosition(this)
	name = GetObjectName(this)
	dmg_radius = GetFieldValue(this, "m_damageRadius")
	if dmg_radius == 4 then
		RIBBON = CreateObject(this, "ModelObject", "ribbon")
	end
	if dmg_radius == 2 then
		RIBBON = CreateObject(this, "ModelObject", "ribbon_2")
	end
	if dmg_radius == 8 then
		RIBBON = CreateObject(this, "ModelObject", "ribbon_8")
	end
	SetFieldValue(RIBBON:GetObject(), "Name", name.."_rib")
	SetWorldPosition(RIBBON:GetObject(), pos)
end

function onExploded()
	dmg_radius = GetFieldValue(this, "m_damageRadius")
	dmg_type = GetFieldValue(this, "m_damageType")
	dmg_value = GetFieldValue(this, "m_damage") * damage_mul

	objects_in_radius = FindObjectsInRadius(GetWorldPosition(this), dmg_radius)
	
	for i,v in ipairs(objects_in_radius) do
		dmg_direction = (GetWorldPosition(v:GetObject()) - GetWorldPosition(this)):normalized()
		dmg_position = GetWorldPosition(v:GetObject())		
		damaged_object = v
		GiveDamage(this, damaged_object, dmg_type, dmg_value, dmg_direction, dmg_position)
	end
end

function onUpdate()
	pos = GetWorldPosition(this)
	SetWorldPosition(RIBBON:GetObject(), pos)
	if explode and ((explosion_time + explosion_delay) < GetTime()) then Explode(this) end
end

function onDestroy()
	DestroyObject(RIBBON:GetObject())
end