-- Script executed for every scripted GameObject before running custom object script

-- Returns current game time in seconds
function GetTime()
    time_ms = GameObject.GetTimeMs(this)
    return time_ms * 0.001
end

-- Finds object on the same level with specified name and class
function FindObjectExactClass(class_name, object_name)
    return GameObject.FindObjectOnLevelByClassAndName(this, class_name, object_name)
end

-- Finds object on the same level with specified name
function FindObject(object_name)
    return GameObject.FindObjectOnLevelByClassAndName(this, "", object_name)
end

-- Compatibility with old API
SetWorldPosition = ModelObject.SetWorldPosition
GetWorldPosition = ModelObject.GetWorldPosition
GetTimeDelta     = GameObject.GetTimeDelta

LogInfo         = GameObject.LogInfo
LogWarning      = GameObject.LogWarning
LogError        = GameObject.LogError
CreateObject    = GameObject.CreateObject
GetTimeMs       = GameObject.GetTimeMs
GetObjectName   = GameObject.GetObjectName
GiveDamage      = GameObject.GiveDamage
DestroyObject   = GameObject.DestroyObject

IsDerivedOrExactClass = GameObject.IsDerivedOrExactClass
CallScriptMethod = GameObject.CallScriptMethod
FindObjectsInRadius = GameObject.FindObjectsInRadius

GetCurrentCameraForwardDir = GameObject.GetCurrentCameraForwardDir
GetCurrentCameraWorldPos   = GameObject.GetCurrentCameraWorldPos