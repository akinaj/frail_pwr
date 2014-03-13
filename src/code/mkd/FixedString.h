/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once

template <unsigned N>
class FixedString
{
public:
    FixedString() { *m_data = 0; }
    FixedString(const char* str)
    {
        assign(str);
    }
    FixedString(const char* str, int len)
    {
        assign(str, len);
    }

    const char* c_str() const { return m_data; }
    
    const char* begin() { return m_data; }
    const char* begin() const { return m_data; }

    const char* end() { return m_data + N; }
    const char* end() const { return m_data + N; }

    size_t size() const { return N; }

    void assign(const char* str)
    {
        MK_ASSERT(str != 0);
        MK_ASSERT(strlen(str) < N);
        strcpy(m_data, str);
    }

    void assign(const char* str, int len)
    {
        MK_ASSERT(len <= N);
        for (int i = 0; i < len-1; ++i)
        {
            m_data[i] = str[i];
        }
        m_data[len-1] = 0;
    }

    int compare(const char* cstr) const
    {
        return strcmp(c_str(), cstr);
    }

    bool operator==(const FixedString& other) const
    {
        return (compare(other.c_str()) == 0);
    }

    bool operator!=(const FixedString& other) const
    {
        return (compare(other.c_str()) != 0);
    }


private:
    char m_data[N];
};

typedef FixedString<32> mkFixedString;
