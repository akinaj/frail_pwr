#include "pch.h"
#include "PresetMgr.h"
#include "Filesystem.h"
#include "serialization/SerializationJSON.h"
#include "rtti/TypeManager.h"
#include "rtti/FieldInfo.h"
#include "rtti/TypeInfo.h"

void rtti::PresetMgr::init()
{
    const size_t type_count = TypeManager::getInstance().getTypeInfoCount();
    for (size_t i = 0; i < type_count; ++i)
    {
        TypeInfo* info = TypeManager::getInstance().getTypeInfoByIdx(i);

        mkString filename = "data/presets/";
        filename += info->getClassName();
        filename += ".json";

        if (fileExists(filename.c_str()))
            readPresetsFromFile(filename);
    }

    bindInheritedPresets();
}

template <typename T>
class AllocatedBuffGuard
{
public:
    explicit AllocatedBuffGuard(T* buff)
        : m_buff(buff)
    {

    }

    ~AllocatedBuffGuard()
    {
        delete[] m_buff;
    }

private:
    T* m_buff;
};

// Reads values for type's fields and repeats for superclass
void loadPresetValues(const rtti::TypeInfo* type, rtti::SPreset& preset, JSONDataReader& reader)
{
    for (size_t i = 0; i < type->getFieldCount(); ++i)
    {
        const rtti::FieldInfo* field = type->getFieldByIdx(i);

        rtti::SPresetEntry entry;
        if (reader.readString(field->getName(), entry.value))
        {
            entry.field = field;
            preset.entries.push_back(entry);
        }
    }

    if (type->getSuperClass())
        loadPresetValues(type->getSuperClass(), preset, reader);
}

void rtti::PresetMgr::readPresetsFromFile( const mkString& filename )
{
    uint8* out_buf_ptr = NULL;
    uint32 out_buf_size = 0;
    if (!loadFile(filename.c_str(), &out_buf_ptr, &out_buf_size))
    {
        log_error("Could not load preset file '%s'", filename.c_str());
        return;
    }

    AllocatedBuffGuard<uint8> buff_guard(out_buf_ptr);

    JSONDataReaderString reader((char*)out_buf_ptr, true);

    if (!reader.isValid())
    {
        log_error("Preset file '%s' contained syntax error and will not be loaded", filename.c_str());
        return;
    }

    mkString class_name;
    if (!reader.readString("Class", class_name))
    {
        log_error("Error in preset file '%s': preset file should start with class name", filename.c_str());
        return;
    }

    TypeInfo* type = TypeManager::getInstance().getTypeInfoByName(class_name);
    if (!type)
    {
        log_error("Error in preset file '%s': class '%s' does not exist", filename.c_str(), class_name.c_str());
        return;
    }

    TClassPresetVec& class_presets_vec = m_presets[type];

    while (reader.readStartBlock("Preset"))
    {
        JSONDataReader preset_reader;
        preset_reader.init(reader.getCurrentBlockValue(), false);

        SPreset preset;
        if (!preset_reader.readString("PresetName", preset.preset_name))
        {
            // For compatibility with old preset files
            if (!preset_reader.readString("Name", preset.preset_name))
            {
                log_error("Error in preset file '%s': preset without name", filename.c_str());
                continue;
            }
        }

        preset_reader.readString("Extends", preset.super_preset_name);

        loadPresetValues(type, preset, preset_reader);

        if (preset.entries.empty())
            log_warning("Preset '%s' in preset file '%s' has no entries", preset.preset_name.c_str(), filename.c_str());

        class_presets_vec.push_back(preset);

        reader.readEndBlock("Preset");
    }
}

bool rtti::PresetMgr::applyPreset( IRTTIObject* obj, const mkString& preset_name ) const
{
    const SPreset* exact_preset = findPreset(obj->getTypeInfo(), preset_name);
    if (!exact_preset)
    {
        log_error("Could not apply preset '%s' to object of class '%s'",
            preset_name.c_str(), obj->getTypeInfo()->getClassName());
        return false;
    }

    // Apply all base presets in reverse order (first base presets, than this one)
    std::vector<const SPreset*> presets_to_apply;
    const SPreset* next_preset = exact_preset->super_preset;
    size_t num_base_presets = 0;
    while (next_preset)
    {
        presets_to_apply.push_back(next_preset);
        next_preset = next_preset->super_preset;
        ++num_base_presets;

        if (num_base_presets > 64)
        {
            log_error("Preset '%s' of class '%s' is on higher than 64 level of inheritance, possible loop! Aborting base presets collection",
                preset_name.c_str(), obj->getTypeInfo()->getClassName());
            break;
        }
    }

    presets_to_apply.insert(presets_to_apply.begin(), exact_preset);

    for (size_t preset_idx = presets_to_apply.size(); preset_idx > 0; --preset_idx)
    {
        const SPreset* preset = presets_to_apply[preset_idx - 1];

        for (size_t i = 0; i < preset->entries.size(); ++i)
        {
            const SPresetEntry& entry = preset->entries[i];        
            entry.field->setValueInObjectFromString(*obj, entry.value);
        }
    }

    return true;
}

const rtti::SPreset* rtti::PresetMgr::findPreset( const rtti::TypeInfo* type_info, const mkString& preset_name ) const
{
    TPresetVecMap::const_iterator preset_vec_it = m_presets.find(type_info);
    if (preset_vec_it == m_presets.end())
    {
        log_error("Could not find preset '%s' of class '%s': class has no presets.",
            preset_name.c_str(), type_info->getClassName());

        return NULL;
    }

    const TClassPresetVec& class_presets_vec = preset_vec_it->second;
    for (size_t i = 0; i < class_presets_vec.size(); ++i)
    {
        const SPreset& preset = class_presets_vec[i];
        if (preset.preset_name == preset_name)
            return &preset;
    }

    log_error("Could not find preset '%s' of class '%s': none of %d presets for this class has such name.",
        preset_name.c_str(), type_info->getClassName(), class_presets_vec.size());

    return NULL;
}

void rtti::PresetMgr::bindInheritedPresets()
{
    for (TPresetVecMap::iterator class_iter = m_presets.begin(); class_iter != m_presets.end(); ++class_iter)
    {
        TClassPresetVec& class_presets = class_iter->second;

        for (TClassPresetVec::iterator preset_iter = class_presets.begin(); preset_iter != class_presets.end(); ++preset_iter)
        {
            SPreset& preset = *preset_iter;

            if (!preset.super_preset_name.empty())
            {
                if (preset.super_preset_name == preset.preset_name)
                {
                    log_error("Preset '%s' of class '%s' inherits itself!",
                        preset.preset_name.c_str(), class_iter->first->getClassName());
                    continue;
                }

                SPreset* super_preset_ptr = NULL;
                for (size_t i = 0; i < class_presets.size(); ++i)
                {
                    if (class_presets[i].preset_name == preset.super_preset_name)
                    {
                        super_preset_ptr = &class_presets[i];
                        break;
                    }
                }

                if (super_preset_ptr == NULL)
                {
                    log_error("Preset '%s' of class '%s' inherits nonexistant preset '%s'", 
                        preset.preset_name.c_str(), class_iter->first->getClassName(), preset.super_preset_name.c_str());
                }
                else
                {
                    preset.super_preset = super_preset_ptr;
                }
            }
        }
    }
}
