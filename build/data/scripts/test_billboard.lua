base_forward = mkVec3.new(0, 0, 1)

function onUpdate()
    awanted_forward = (ModelObject.GetWorldPosition(this) - GetCurrentCameraWorldPos()):normalized()
    wanted_forward = mkVec3.new(awanted_forward:x(), 0, awanted_forward:z())
    wanted_orientation = mkVec3.rotation_to(base_forward, wanted_forward)
	ModelObject.SetOrientation(this, wanted_orientation)
end
