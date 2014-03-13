/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#pragma once
#include "Serialization.h"
#include "json.h"

class JSONDataReader : public IDataReader
{
public:
    JSONDataReader();

    bool init(json_value* root, bool is_sequential);

    bool isValid() const;

    virtual bool readInt32(const char* name, int32& out);
    virtual bool readUInt32(const char* name, uint32& out);
    virtual bool readFloat(const char* name, float& out);
    virtual bool readString(const char* name, mkString& out);
    virtual bool readBytes(const char* name, size_t num, uint8* out_buff);

    virtual bool peekInt32(const char* name, int32& out);
    virtual bool peekUInt32(const char* name, uint32& out);
    virtual bool peekFloat(const char* name, float& out);
    virtual bool peekString(const char* name, mkString& out);
    virtual bool peekBytes(const char* name, size_t num, uint8* out_buff);

    virtual bool readStartBlock(const char* name);
    virtual bool readEndBlock(const char* name);

    // If current node is a block, returns its name. Note that for sequential read,
    // if (getNextBlockName()) readStartBlock(getNextBlockName()); always succeed
    virtual const char* getNextBlockName() const;

    json_value* getCurrentBlockValue();

private:
    bool m_sequential;
    json_value* m_currBlockRoot;
    json_value* m_currentNode;

    void nextNode();
    bool moveToNamedNode(const char* name);
    json_value* findChild(json_value* of, const char* name);
};

class JSONDataWriter : public IDataWriter
{
public:
    JSONDataWriter();

    virtual bool writeInt32(const char* name, int32 out);
    virtual bool writeUInt32(const char* name, uint32 out);
    virtual bool writeFloat(const char* name, float out);
    virtual bool writeString(const char* name, const mkString& out);
    virtual bool writeBytes(const char* name, size_t num, const uint8* const out_buff);

    virtual bool writeStartBlock(const char* name);
    virtual bool writeEndBlock();

    json_value* getRoot() const;

protected:
    block_allocator m_allocator;
    json_value* m_currBlockRoot;
    json_value* m_currNode;

    json_value* createNextSibling(const char* name);
    json_value* createJSONValue();
};

class JSONDataReaderString : public JSONDataReader
{
public:
    JSONDataReaderString(const char* data, bool sequential);

private:
    block_allocator m_allocator;
};

class JSONDataWriterString : public JSONDataWriter
{
public:
    JSONDataWriterString();

    void extractString(mkString& out_str);
    
    // Returned pointer will be valid for a lifetime of this object
    const char* getString();

private:
    const char* m_lastExtractedString;
    json_value* m_lastExtractionCurrNode;

    bool isDirty() const;
    void recreateExtractedString();
};
