/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once

class IDataReader
{
public:
    virtual bool readInt32(const char* name, int32& out) = 0;
    virtual bool readUInt32(const char* name, uint32& out) = 0;
    virtual bool readFloat(const char* name, float& out) = 0;
    virtual bool readString(const char* name, mkString& out) = 0;
    virtual bool readBytes(const char* name, size_t num, uint8* out_buff) = 0;

    virtual bool peekInt32(const char* name, int32& out) = 0;
    virtual bool peekUInt32(const char* name, uint32& out) = 0;
    virtual bool peekFloat(const char* name, float& out) = 0;
    virtual bool peekString(const char* name, mkString& out) = 0;
    virtual bool peekBytes(const char* name, size_t num, uint8* out_buff) = 0;

    virtual bool readStartBlock(const char* name) = 0;
    virtual bool readEndBlock(const char* name) = 0;

    virtual const char* getNextBlockName() const = 0;
};

class IDataWriter
{
public:
    virtual bool writeInt32(const char* name, int32 out) = 0;
    virtual bool writeUInt32(const char* name, uint32 out) = 0;
    virtual bool writeFloat(const char* name, float out) = 0;
    virtual bool writeString(const char* name, const mkString& out) = 0;
    virtual bool writeBytes(const char* name, size_t num, const uint8* const out_buff) = 0;

    virtual bool writeStartBlock(const char* name) = 0;
    virtual bool writeEndBlock() = 0;
};
