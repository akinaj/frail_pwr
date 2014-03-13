/***********************************
* mkdemo 2011-2013                *
* author: Maciej Kurowski 'kurak' *
***********************************/
#include "pch.h"
#include "utils.h"

mkVec3 getRandomHorizontalDir()
{
    // Make vector from origin to random point in square (-1, 1)-(1, -1)
    float x = (rand() / (float)RAND_MAX) * 2.0f - 1.0f;
    float z = (rand() / (float)RAND_MAX) * 2.0f - 1.0f;

    mkVec3 vec(x, 0, z);
    vec.normalise();

    return vec;
}

bool are_strings_equal_case_insensitive(const mkString& lhs, const mkString& rhs)
{
    if (lhs.size() != rhs.size())
        return false;

    for (size_t i = 0; i < lhs.size(); ++i)
    {
        const char lch = lhs[i];
        const char rch = rhs[i];

        if (lch != rch && tolower(lch) != tolower(rch))
            return false;
    }

    return true;
}

Ogre::Vector3 bullet_to_ogre(const btVector3& vec)
{
    return Ogre::Vector3(vec.x(), vec.y(), vec.z());
}

Ogre::Quaternion bullet_to_ogre(const btQuaternion& quat)
{
    return Ogre::Quaternion(quat.w(), quat.x(), quat.y(), quat.z());
}

const btVector3 ogre_to_bullet(const Ogre::Vector3& vec)
{
    return btVector3(vec.x, vec.y, vec.z);
}

btQuaternion ogre_to_bullet( const mkQuat& quat )
{
    return btQuaternion(quat.x, quat.y, quat.z, quat.w);
}

mkVec3 get_vec3( json_value* val )
{
    MK_ASSERT(val->type == JSON_ARRAY);
    mkVec3 result;
    json_value* coord = val->first_child;

#define GET_NEXT_COORD(out) MK_ASSERT(coord); MK_ASSERT(coord->type == JSON_FLOAT); out = coord->float_value; coord = coord->next_sibling;

    GET_NEXT_COORD(result.x);
    GET_NEXT_COORD(result.y);
    GET_NEXT_COORD(result.z);

#undef GET_NEXT_COORD

    return result;
}

int get_int(json_value* val) { MK_ASSERT(val->type == JSON_INT); return val->int_value; }
bool get_bool(json_value* val) { MK_ASSERT(val->type == JSON_BOOL); return val->int_value != 0; }
const char* get_string(json_value* val) { MK_ASSERT(val->type == JSON_STRING); return val->string_value; }
float get_float(json_value* val) { MK_ASSERT(val->type == JSON_FLOAT); return val->float_value; }

float randFloat()
{
    return rand() / (float)RAND_MAX;
}

float randFloat( float minval, float maxval )
{
    const float range = maxval - minval;
    return randFloat() * range + minval;
}

mkVec3 slerp_horz_direction( float t, const mkVec3& start_dir, const mkVec3& dest_dir )
{
    const mkQuat base_dir_quat(start_dir, mkVec3::UNIT_Y, start_dir.crossProduct(mkVec3::UNIT_Y));
    const mkQuat dest_dir_quat(dest_dir, mkVec3::UNIT_Y, dest_dir.crossProduct(mkVec3::UNIT_Y));
    const mkQuat cur_dir_quat = mkQuat::Slerp(t, base_dir_quat, dest_dir_quat, true);
    mkVec3 axis_x, axis_y, axis_z;
    cur_dir_quat.ToAxes(axis_x, axis_y, axis_z);
    return axis_x;
}

void log_printf_impl( int category, const char* func_name, const int line_no, const char* fmt_str, ... )
{
    time_t ttt = time(NULL);
    const tm* times = localtime(&ttt);

    char buff[2048] = {0};

    va_list ap;
    va_start(ap, fmt_str);
    vsnprintf(buff, _countof(buff), fmt_str, ap);
    va_end(ap);

    char func_info[1024] = {0};
    sprintf(func_info, "[%s:%d] ", func_name, line_no);

    OutputDebugString(func_info);
    OutputDebugString(buff);
    OutputDebugString("\n");

    if (category == 'ERR ')
    {
        const int ret_val = MessageBoxA(NULL, buff, func_info, MB_ABORTRETRYIGNORE | MB_ICONERROR);
        switch (ret_val)
        {
        case IDABORT: ExitProcess(-1); break;
        case IDRETRY: MK_HALT(); break;
        case IDIGNORE: break;
        }
    }
}

mkVec3 rotate_horz_vec( const mkVec3& in_vec, const float angle_rad )
{
    mkMat3 mtx;
    mtx.FromAxisAngle(mkVec3::UNIT_Y, Ogre::Radian(angle_rad));
    return in_vec * mtx;
}

mkString int2str( int val )
{
    return Ogre::StringConverter::toString(val);
}

mkString float2str( float val )
{
    return Ogre::StringConverter::toString(val);
}

bool string_starts_with( const char* str, const char* with )
{
    while (*with != '\0')
    {
        // str is shorter, failed
        if (*str == '\0')
            return false;

        if (*str != *with)
            return false;

        ++str;
        ++with;
    }

    return true;
}

mkString vec32str( const mkVec3& val )
{
    return Ogre::StringConverter::toString(val);
}

mkString quat2str( const mkQuat& val )
{
    // TODO: consider axis-angle instead wxyz
    return Ogre::StringConverter::toString(val);
}

mkString mat42str( const mkMat4& val )
{
    return Ogre::StringConverter::toString(val);
}

mkVec3 transform_point( const mkMat4& transform, const mkVec3& point )
{
    return transform.transformAffine(point); // transforms vec4(point, 1) and returns xyz

}

mkVec3 transform_dir( const mkMat4& transform, const mkVec3& dir )
{
    Ogre::Vector4 dir_vec(dir);
    dir_vec.w = 0.f;
    Ogre::Vector4 transformed_dir = transform.transformAffine(dir_vec);

    return mkVec3(transformed_dir.x, transformed_dir.y, transformed_dir.z);
}

//////////////////////////////////////////////////////////////////////////

bool equal(aiVariant& first, aiVariant& second, bool& result){
    float *f1 = boost::get<float>(&first);
    bool *b1 = boost::get<bool>(&first);

    if(f1){
        float *f2 = boost::get<float>(&second);
        result = (*f1 == *f2);
        return true;
    } else if(b1){
        bool *b2 = boost::get<bool>(&second);
        result = (*b1 == *b2);
        return true;
    }
    return false;
}

bool notEqual(aiVariant& first, aiVariant& second, bool& result){
    float *f1 = boost::get<float>(&first);
    bool *b1 = boost::get<bool>(&first);

    if(f1){
        float *f2 = boost::get<float>(&second);
        result = (*f1 != *f2);
        return true;
    } else if(b1){
        bool *b2 = boost::get<bool>(&second);
        result = (*b1 != *b2);
        return true;
    }
    return false;
}

bool more(aiVariant& first, aiVariant& second, bool& result){
    float *f1 = boost::get<float>(&first);
    bool *b1 = boost::get<bool>(&first);

    if(f1){
        float *f2 = boost::get<float>(&second);
        result = (*f1 > *f2);
        return true;
    } else if(b1){
        bool *b2 = boost::get<bool>(&second);
        result = (*b1 > *b2);
        return true;
    }
    return false;
}

bool moreEqual(aiVariant& first, aiVariant& second, bool& result){
    float *f1 = boost::get<float>(&first);
    bool *b1 = boost::get<bool>(&first);

    if(f1){
        float *f2 = boost::get<float>(&second);
        result = (*f1 >= *f2);
        return true;
    } else if(b1){
        bool *b2 = boost::get<bool>(&second);
        result = (*b1 >= *b2);
        return true;
    }
    return false;
}

bool less(aiVariant& first, aiVariant& second, bool& result){
    float *f1 = boost::get<float>(&first);
    bool *b1 = boost::get<bool>(&first);

    if(f1){
        float *f2 = boost::get<float>(&second);
        result = (*f1 < *f2);
        return true;
    } else if(b1){
        bool *b2 = boost::get<bool>(&second);
        result = (*b1 < *b2);
        return true;
    }
    return false;
}

bool lessEqual(aiVariant& first, aiVariant& second, bool& result){
    float *f1 = boost::get<float>(&first);
    bool *b1 = boost::get<bool>(&first);

    if(f1){
        float *f2 = boost::get<float>(&second);
        result = (*f1 <= *f2);
        return true;
    } else if(b1){
        bool *b2 = boost::get<bool>(&second);
        result = (*b1 <= *b2);
        return true;
    }
    return false;
}

time_t lastModifiedTime(const std::string& filePath){
    struct tm* clock;
    struct stat attrib;
    stat(filePath.c_str(), &attrib);
    clock = gmtime(&(attrib.st_mtime));
    return mktime(clock);
}