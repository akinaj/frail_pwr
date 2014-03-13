 -- Script for double damage. Author: Sebastian Łasisz

angle = 1

function onTouched(aktor)	
	if IsDerivedOrExactClass(aktor:GetObject(), "Player") then
		SetFieldValue(aktor:GetObject(), "m_shootingDamage", GetFieldValue(aktor:GetObject(), "m_shootingDamage") * 2)		
		bilboard = CreateObject(this, "ModelObject", "bilb_2")
		name = GetObjectName(this)
		SetFieldValue(bilboard:GetObject(), "Name", "bilb"..name)
		SetWorldPosition(bilboard:GetObject(), (GetWorldPosition(aktor:GetObject())+mkVec3.new(0,2.1,0)))			
		DestroyObject(this)
	end		
end

function onUpdate()
	angle = angle + 150 * GetTimeDelta()
	ModelObject.SetOrientation(this, mkQuat.new(mkVec3.new(0, 1, 0), angle))
end
