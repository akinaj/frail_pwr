#pragma once

namespace rtti
{

    class IRTTIObject;
    class TypeInfo;
    class FieldInfo;

    struct SPresetEntry
    {
        const FieldInfo* field;
        mkString value;
    };

    struct SPreset
    {
        mkString super_preset_name;
        SPreset* super_preset;

        mkString preset_name;

        typedef std::vector<SPresetEntry> TEntryVec;
        TEntryVec entries;

        SPreset()
            : super_preset(NULL) { }
    };

    class PresetMgr
    {
    public:
        void init();

        const SPreset* findPreset(const TypeInfo* type_info, const mkString& preset_name) const;
        bool applyPreset(IRTTIObject* obj, const mkString& preset_name) const;

    private:
        typedef std::vector<SPreset> TClassPresetVec;
        typedef std::map<const TypeInfo*, TClassPresetVec> TPresetVecMap;
        TPresetVecMap m_presets;

        // Presets can be added only on initialization, as later pointers to vector elements are used

        void readPresetsFromFile(const mkString& filename);
        void bindInheritedPresets();
    };

}
