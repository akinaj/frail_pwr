/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include <hash_map>
#include "FixedString.h"
#include <json.h>

using stdext::hash_map;

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                              \
                ((DWORD)(BYTE)(ch0)    | ((DWORD)(BYTE)(ch1) << 8) |   \
                ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24 ))
#endif

const btVector3 ogre_to_bullet(const mkVec3&);
mkVec3 bullet_to_ogre(const btVector3&);
Ogre::Quaternion bullet_to_ogre(const btQuaternion& quat);
btQuaternion ogre_to_bullet(const mkQuat& quat);
mkVec3 getRandomHorizontalDir();
float randFloat();
float randFloat(float minval, float maxval);

template <typename T>
T clamp(T val, T min_val, T max_val)
{
    if (val < min_val)
        return min_val;
    else if (val > max_val)
        return max_val;
    else
        return val;
}

bool string_starts_with(const char* str, const char* with);
bool are_strings_equal_case_insensitive(const mkString& lhs, const mkString& rhs);

int get_int(json_value* val);
bool get_bool(json_value* val);
const char* get_string(json_value* val);
float get_float(json_value* val);
mkVec3 get_vec3(json_value* val);

mkVec3 slerp_horz_direction(float t, const mkVec3& start_dir, const mkVec3& dest_dir);

void log_printf_impl(int category, const char* func_name, const int line_no, const char* fmt_str, ...);

#define log_trace() log_printf_impl('TRAC', __FUNCTION__, __LINE__, "[trace]")
#define log_trace_method() log_printf_impl('TRAC', __FUNCTION__, __LINE__, "[trace] obj: %p", this)
#define log_info(fmt_str, ...) log_printf_impl('INFO', __FUNCTION__, __LINE__, (fmt_str), ##__VA_ARGS__)
#define log_error(fmt_str, ...) log_printf_impl('ERR ', __FUNCTION__, __LINE__, (fmt_str), ##__VA_ARGS__)
#define log_warning(fmt_str, ...) log_printf_impl('WARN', __FUNCTION__, __LINE__, (fmt_str), ##__VA_ARGS__)

mkVec3 rotate_horz_vec(const mkVec3& in_vec, const float angle_rad);

mkString int2str(int val);
mkString float2str(float val);
mkString vec32str(const mkVec3& val);
mkString quat2str(const mkQuat& val);
mkString mat42str(const mkMat4& val);

mkVec3 transform_point(const mkMat4& transform, const mkVec3& point);
mkVec3 transform_dir(const mkMat4& transform, const mkVec3& dir);

template <typename T>
void fast_remove_from_vec(std::vector<T>& vec, const size_t idx_to_remove)
{
    MK_ASSERT(idx_to_remove < vec.size());
    vec[idx_to_remove] = vec.back();
    vec.resize(vec.size() - 1);
}

template <typename T>
bool fast_remove_val_from_vec(std::vector<T>& vec, T val)
{
    bool result = false;
    for (size_t i = 0; i < vec.size(); )
    {
        if (vec[i] == val)
        {
            fast_remove_from_vec(vec, i);
            result = true;
        }
        else
        {
            ++i;
        }
    }

    return result;
}

// Replaces first found orig_val in vec to new_val and returns its index
// If no such value was found, vec size is returned
template <typename T>
size_t replace_first_val_in_vec(std::vector<T>& vec, T orig_val, T new_val)
{
    for (size_t i = 0; i < vec.size(); ++i)
    {
        if (vec[i] == orig_val)
        {
            vec[i] = new_val;
            return i;
        }
    }

    return vec.size();
}

// Returns index of last element not equal to specified
// If no such element exists, return vec size
template <typename IterType, typename ValueType>
size_t last_not_equal_to_idx(IterType begin, IterType end, ValueType value)
{
    size_t size = end - begin;
    if (size == 0)
        return 0;

    size_t idx = size - 1;
    while (idx > 0)
    {
        if (!(*(begin + idx) == value))
            return idx;

        --idx;
    }

    if (!(*begin == value))
        return 0;
    else
        return size;
}

template <typename T>
size_t last_not_equal_to_idx(const std::vector<T>& vec, T value)
{
    return last_not_equal_to_idx(vec.begin(), vec.end(), value);
}

#define STATIC_ASSERT(condition) typedef char static_assert_failed[(condition) ? 1 : -1];

template <typename T, int N>
void fill_array(T (&array_to_fill)[N], T val)
{
    for (int i = 0; i < N; ++i)
        array_to_fill[i] = val;
}

template <typename T, int N>
void zero_array(T (&array_to_fill)[N])
{
    fill_array<T, N>(array_to_fill, 0);
}

time_t lastModifiedTime(const std::string& filePath);

//////////////////////////////////////////////////////////////////////////

typedef boost::variant<float, bool, mkVec3> aiVariant;
bool equal(aiVariant& first, aiVariant& second, bool& result);
bool notEqual(aiVariant& first, aiVariant& second, bool& result);
bool more(aiVariant& first, aiVariant& second, bool& result);
bool moreEqual(aiVariant& first, aiVariant& second, bool& result);
bool less(aiVariant& first, aiVariant& second, bool& result);
bool lessEqual(aiVariant& first, aiVariant& second, bool& result);