/***********************************
 * mkdemo 2011-2013                *
 * author: Maciej Kurowski 'kurak' *
 ***********************************/
#include "pch.h"
#include "SerializationJSON.h"
#include "utils.h"

////////////////////////////////////////////////////////////////////////////////////////
JSONDataReader::JSONDataReader()
    : m_currBlockRoot(NULL)
    , m_currentNode(NULL)
{

}

bool JSONDataReader::init( json_value* root, bool is_sequential )
{
    if (m_currBlockRoot)
        return false;

    m_currBlockRoot = root;
    m_currentNode = m_currBlockRoot->first_child;
    m_sequential = is_sequential;

    return true;
}

bool JSONDataReader::isValid() const
{
    return m_currBlockRoot != NULL;
}

#define READ(val) if (!val) { return false; } nextNode(); return true;

bool JSONDataReader::readInt32(const char* name, int32& out)
{
    READ( peekInt32(name, out) );
}

bool JSONDataReader::readUInt32(const char* name, uint32& out)
{
    READ( peekUInt32(name, out) );
}

bool JSONDataReader::readFloat(const char* name, float& out)
{
    READ( peekFloat(name, out) );
}

bool JSONDataReader::readString(const char* name, mkString& out)
{
    READ( peekString(name, out) );
}

bool JSONDataReader::readBytes(const char* name, size_t num, uint8* out_buff)
{
    READ( peekBytes(name, num, out_buff) );
}

#undef READ 

bool JSONDataReader::peekInt32(const char* name, int32& out)
{
    if (!m_currentNode || !moveToNamedNode(name) || m_currentNode->type != JSON_INT)
        return false;

    out = m_currentNode->int_value;
    return true;
}

bool JSONDataReader::peekUInt32(const char* name, uint32& out)
{
    if (!m_currentNode || !moveToNamedNode(name) || m_currentNode->type != JSON_INT)
        return false;

    out = (uint32)m_currentNode->int_value;
    return true;
}

bool JSONDataReader::peekFloat(const char* name, float& out)
{
    if (!m_currentNode || !moveToNamedNode(name) || (m_currentNode->type != JSON_FLOAT && m_currentNode->type != JSON_INT))
        return false;

    if (m_currentNode->type == JSON_FLOAT)
        out = m_currentNode->float_value;
    else
        out = (float)m_currentNode->int_value;

    return true;
}

bool JSONDataReader::peekString(const char* name, mkString& out)
{
    if (!m_currentNode || !moveToNamedNode(name) || m_currentNode->type != JSON_STRING)
        return false;

    out = m_currentNode->string_value;
    return true;
}

bool JSONDataReader::peekBytes(const char* name, size_t num, uint8* out_buff)
{
    if (!m_currentNode || !moveToNamedNode(name) || m_currentNode->type != JSON_ARRAY)
        return false;

    json_value* array_val = m_currentNode->first_child;
    size_t items_loaded = 0;

    while (array_val != NULL && items_loaded < num)
    {
        if (array_val->type != JSON_INT)
        {
            log_error("Serialized byte array %s contains non-int elements of type %d on index %d", name, array_val->type, (int)items_loaded);
            return false;
        }

        if (array_val->int_value > std::numeric_limits<uint8>::max() || array_val->int_value < 0)
        {
            log_error("Serialized byte array %s contains element with value exceeding byte: %d on index %d", name, array_val->int_value, (int)items_loaded);
            return false;
        }

        out_buff[items_loaded] = (uint8)array_val->int_value;

        ++items_loaded;
        array_val = array_val->next_sibling;
    }

    if (items_loaded < num)
    {
        log_error("Serialized byte array %s contained only %d elements, while %d was required", name, (int)items_loaded, (int)num);
        return false;
    }

    if (array_val != NULL)
        log_warning("Serialized byte array %d contained more than %d elements!", name, (int)num);

    return true;
}

void JSONDataReader::nextNode()
{
    MK_ASSERT(m_currentNode);

    m_currentNode = m_currentNode->next_sibling;

    if (!m_sequential && m_currentNode == NULL)
        m_currentNode = m_currBlockRoot->first_child;
}

bool JSONDataReader::moveToNamedNode( const char* name )
{
    if (m_currBlockRoot == NULL || m_currentNode == NULL)
        return false;

    // NULL name means "just get next value" so we can ignore name check
    if (name == NULL)
        return true;
    // For sequential access just check current node name
    else if (m_sequential)
    {
        if (m_currentNode->name == NULL) // NULL node name with not NULL arg - check failed
            return false;

        return strcmp(name, m_currentNode->name) == 0;
    }
    // Non-sequential access with name check
    else
    {
        json_value* node = findChild(m_currBlockRoot, name);
        if (node != NULL)
        {
            m_currentNode = node;
            return true;
        }
    }

    return false;
}

const char* JSONDataReader::getNextBlockName() const
{
    if (m_currentNode && m_currentNode->type == JSON_OBJECT)
        return m_currentNode->name;

    return NULL;
}

bool JSONDataReader::readStartBlock( const char* name )
{
    if (moveToNamedNode(name))
    {
        m_currBlockRoot = m_currentNode;
        m_currentNode = m_currBlockRoot->first_child;
        return true;
    }

    return false;
}

bool JSONDataReader::readEndBlock( const char* name )
{
    if (m_currBlockRoot->name == NULL || strcmp(m_currBlockRoot->name, name) != 0)
        return false;

    if (m_currBlockRoot->parent == NULL)
        return false;

    m_currentNode = m_currBlockRoot;
    m_currBlockRoot = m_currBlockRoot->parent;

    nextNode(); // this handles rewinding to beginning for non-sequential reader

    return true;
}

json_value* JSONDataReader::findChild( json_value* of, const char* name )
{
    for (json_value* child = of->first_child; child != NULL; child = child->next_sibling)
    {
        if (child->name && strcmp(child->name, name) == 0)
            return child;
    }

    return NULL;
}

json_value* JSONDataReader::getCurrentBlockValue()
{
    return m_currBlockRoot;
}

////////////////////////////////////////////////////////////////////////////////////////

JSONDataWriter::JSONDataWriter()
    : m_allocator(64)
{
    m_currNode = NULL;

    m_currBlockRoot = createJSONValue();
    m_currBlockRoot->type = JSON_OBJECT;
    m_currBlockRoot->name = NULL;
}

json_value* JSONDataWriter::getRoot() const
{
    json_value* result = m_currBlockRoot;
    json_value* parent = result->parent;

    while (parent != NULL)
    {
        result = parent;
        parent = parent->parent;
    }

    return result;
}

json_value* JSONDataWriter::createNextSibling(const char* name)
{
    json_value* result = createJSONValue();

    if (name)
    {
        const size_t name_len = strlen(name);
        char* name_copy = (char*)m_allocator.malloc(name_len + 1);
        memcpy(name_copy, name, name_len);
        name_copy[name_len] = '\0';

        result->name = name_copy;
    }

    result->parent = m_currBlockRoot;

    if (m_currBlockRoot->last_child == NULL)
        m_currBlockRoot->last_child = m_currBlockRoot->first_child = result;
    else
        m_currBlockRoot->last_child->next_sibling = result;

    m_currBlockRoot->last_child = result;
    
    m_currNode = result;

    return result;
}

json_value* JSONDataWriter::createJSONValue()
{
    json_value* result = (json_value*)m_allocator.malloc(sizeof(json_value));
    memset(result, 0, sizeof(json_value));

    return result;
}

bool JSONDataWriter::writeInt32(const char* name, int32 out)
{
    createNextSibling(name);
    m_currNode->int_value = out;
    m_currNode->type = JSON_INT;

    return true;
}

bool JSONDataWriter::writeUInt32(const char* name, uint32 out)
{
    createNextSibling(name);
    m_currNode->int_value = (int)out;
    m_currNode->type = JSON_INT;

    return true;
}

bool JSONDataWriter::writeFloat(const char* name, float out)
{
    createNextSibling(name);
    m_currNode->float_value = out;
    m_currNode->type = JSON_FLOAT;

    return true;
}

bool JSONDataWriter::writeString(const char* name, const mkString& out)
{
    // Should we validate given string against breaking JSON?

    createNextSibling(name);

    const size_t string_len = out.length();
    if (string_len == 0)
        m_currNode->string_value = "";
    else
    {
        m_currNode->string_value = (char*)m_allocator.malloc(string_len + 1);
        memcpy(m_currNode->string_value, out.c_str(), string_len);
        m_currNode->string_value[string_len] = '\0';
    }

    m_currNode->type = JSON_STRING;

    return true;
}

bool JSONDataWriter::writeBytes(const char* name, size_t num, const uint8* const out_buff)
{
    createNextSibling(name);

    m_currNode->type = JSON_ARRAY;

    if (num > 0)
    {
        for (size_t i = 0; i < num; ++i)
        {
            json_value* array_elem = createJSONValue();
            
            array_elem->parent = m_currNode;
            if (m_currNode->last_child == NULL)
                m_currNode->last_child = m_currNode->first_child = array_elem;
            else
                m_currNode->last_child->next_sibling = array_elem;
            m_currNode->last_child = array_elem;

            array_elem->type = JSON_INT;
            array_elem->int_value = (int)out_buff[i];
        }
    }

    return true;
}

bool JSONDataWriter::writeStartBlock( const char* name )
{
    createNextSibling(name);

    m_currNode->type = JSON_OBJECT;
    m_currBlockRoot = m_currNode;

    return true;
}

bool JSONDataWriter::writeEndBlock()
{
    if (m_currBlockRoot->parent == NULL)
        return false;

    m_currBlockRoot = m_currBlockRoot->parent;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////
JSONDataReaderString::JSONDataReaderString( const char* data, bool sequential )
    : m_allocator(64)
{
    size_t data_len = strlen(data);
    char* data_copy = (char*)m_allocator.malloc(data_len + 1);
    memcpy(data_copy, data, data_len);
    data_copy[data_len] = '\0';

    char* error_pos = NULL;
    char* error_desc = NULL;
    int error_line = 0;

    json_value* root_val = json_parse(data_copy, &error_pos, &error_desc, &error_line, &m_allocator);
    if (root_val == NULL)
        log_error("Error when deserializing from string in line %d: %s", error_line, error_desc);
    else
        init(root_val, sequential);

    // Without call to init, all readX calls will fail (and we're OK with that)
}

////////////////////////////////////////////////////////////////////////////////////////
JSONDataWriterString::JSONDataWriterString()
    : m_lastExtractedString(NULL)
    , m_lastExtractionCurrNode(NULL)
{

}

////////////////////////////////////////////////////////////////////////////////////////
void JSONDataWriterString::extractString( mkString& out_str )
{
    out_str = getString();
}

const char* JSONDataWriterString::getString()
{
    if (isDirty())
        recreateExtractedString();

    return m_lastExtractedString;
}

bool JSONDataWriterString::isDirty() const
{
    return m_lastExtractedString == NULL || m_lastExtractionCurrNode != m_currNode;
}

bool isSimpleJSONVal(json_value* val) /// for formatting only - simple means value without name and children
{
    return val->name == NULL && val->first_child == NULL;
}

static mkString json_val_to_string(json_value* val, const mkString& indent)
{
    mkString child_indent = indent + "    ";
    mkString result;

    result += indent;

    if (val->name != NULL)
    {
        result += "\"";
        result += val->name;
        result += "\" : ";
    }

    switch(val->type)
    {
    case JSON_OBJECT:
        {
            result += "{";

            bool end_with_newline = false;
            for (json_value* child = val->first_child; child != NULL; child = child->next_sibling)
            {
                mkString wanted_indent = child_indent;
                if (isSimpleJSONVal(child))
                {
                    result += " ";
                    wanted_indent = "";
                }
                else
                {
                    result += "\n";
                    end_with_newline |= true;
                }

                result += json_val_to_string(child, wanted_indent);

                if (child->next_sibling)
                    result += ",";
            }

            if (end_with_newline)
                result += "\n" + indent + "}";
            else
                result += " }";
        }
        break;

    case JSON_ARRAY:
        {
            result += "[ ";
            for (json_value* child = val->first_child; child != NULL; child = child->next_sibling)
            {
                result += json_val_to_string(child, "");
                if (child->next_sibling != NULL)
                    result += ", ";
            }
            result += " ]";
        }
        break;

    case JSON_INT:
        result += int2str(val->int_value);
        break;

    case JSON_FLOAT:
        result += float2str(val->float_value);
        break;

    case JSON_BOOL:
        result += val->int_value == 0 ? "false" : "true";
        break;

    case JSON_STRING:
        result += "\"";
        result += val->string_value;
        result += "\"";
        break;

    case JSON_NULL:
        result += "null";
        break;
    }

    return result;
}

// This func is really just a stub
// allocator is just for returning output value, inside is heavily allocating with strings and vectors :/
static char* json_to_string(json_value* val, block_allocator* allocator)
{
    if (val->type != JSON_OBJECT)
        return NULL;

    mkString result_content = json_val_to_string(val, "");
    char* result = (char*)allocator->malloc(result_content.size() + 1);
    memcpy(result, result_content.c_str(), result_content.size());
    result[result_content.size()] = '\0';

    return result;
}

void JSONDataWriterString::recreateExtractedString()
{
    m_lastExtractedString = json_to_string(m_currBlockRoot, &m_allocator);
    m_lastExtractionCurrNode = m_currNode;
}

////////////////////////////////////////////////////////////////////////////////////////

#define EXPECT_VAL_IMPL(val, expected, positive, should) if ((val != expected) == positive) { log_error("Equality test failed: " #val " " should " be " #expected, ", is %s", Ogre::StringConverter::toString(val).c_str()); }
#define EXPECT_VAL_STR_IMPL(val, expected, positive, should) if ((strcmp(val, expected) != 0) == positive) { log_error("Equality test failed: " #val " " should " be %s, is %s", expected, val); }
#define EXPECT_VAL(val, expected) EXPECT_VAL_IMPL(val, expected, true, "should")
#define EXPECT_VAL_STR(val, expected) EXPECT_VAL_STR_IMPL(val, expected, true, "should")
#define EXPECT_DIFF(val, expected) EXPECT_VAL_IMPL(val, expected, false, "should not")
#define EXPECT_DIFF_L_STR(val, expected) EXPECT_VAL_STR_IMPL(val, expected, false, "should not")
class JSONTest
{
public:
    JSONTest()
    {
        mkString written_str;
        uint8 bytes_array_orig[6] = { 1, 2, 5, 10, 255, 0 };
        JSONDataWriterString writer;
        writer.writeInt32("test1", 123);
        writer.writeFloat("test2", 666.192f);
        writer.writeStartBlock("blok1");
        writer.writeString("test3", "bumbum");
        writer.writeUInt32("test4", ~0u);
        writer.writeEndBlock();
        writer.writeBytes("test5", 6, bytes_array_orig);

        writer.extractString(written_str);

        int32 int_val = 0;
        uint32 uint_val = 0;
        float float_val = 0.f;
        mkString string_val;
        uint8 bytes_array[6] = {0};

        JSONDataReaderString reader(written_str.c_str(), true);

        reader.readInt32("test1", int_val);
        reader.readFloat("test2", float_val);
        reader.readStartBlock("blok1");
        reader.readString("test3", string_val);
        reader.readUInt32("test4", uint_val);
        reader.readEndBlock("blok1");
        reader.readBytes("test5", 6, bytes_array);

        EXPECT_VAL(int_val, 123);
        EXPECT_VAL(float_val, 666.192f);
        EXPECT_VAL(uint_val, ~0u);
        EXPECT_VAL_STR(string_val.c_str(), "bumbum");
        EXPECT_VAL_STR((const char*)bytes_array, (const char*)bytes_array_orig);

        EXPECT_DIFF(int_val, 0);
        EXPECT_DIFF(float_val, 0.f);
        EXPECT_DIFF(uint_val, 0u);

        log_info("Test 1 completed");

        JSONDataWriterString writer2;

        writer2.writeInt32("test1", int_val);
        writer2.writeFloat("test2", float_val);
        writer2.writeStartBlock("blok1");
        writer2.writeString("test3", string_val);
        writer2.writeUInt32("test4", uint_val);
        writer2.writeEndBlock();
        writer2.writeBytes("test5", 6, bytes_array);

        mkString extract_str_2;
        writer2.extractString(extract_str_2);

        EXPECT_VAL_STR(extract_str_2.c_str(), written_str.c_str());

        log_info("Test 2 completed");

        {
            JSONDataReaderString reader2(written_str.c_str(), false);

            int32 int_val = 0;
            uint32 uint_val = 0;
            float float_val = 0.f;
            mkString string_val;
            uint8 bytes_array[6] = {0};

            reader2.readFloat("test2", float_val);
            reader2.readStartBlock("blok1");
            reader2.readUInt32("test4", uint_val);
            reader2.readString("test3", string_val);
            reader2.readEndBlock("blok1");
            reader2.readBytes("test5", 6, bytes_array);
            reader2.readInt32("test1", int_val);

            EXPECT_VAL(int_val, 123);
            EXPECT_VAL(float_val, 666.192f);
            EXPECT_VAL(uint_val, ~0u);
            EXPECT_VAL_STR(string_val.c_str(), "bumbum");
            EXPECT_VAL_STR((const char*)bytes_array, (const char*)bytes_array_orig);

            EXPECT_DIFF(int_val, 0);
            EXPECT_DIFF(float_val, 0.f);
            EXPECT_DIFF(uint_val, 0u);

            log_info("Test 3 completed");
        }
    }
} g_JSONTest;
