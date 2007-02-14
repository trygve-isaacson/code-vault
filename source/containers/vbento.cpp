/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vbento.h"

#include "vexception.h"
#include "vbufferedfilestream.h"
#include "vhex.h"

/**
VBentoAttribute is an abstract base class for all of the concrete VBento
attribute classes. Each VBentoNode object in the object hierarchy can
have zero or more attributes; each such attribute exists in memory as
a concrete VBentoAttribute-derived class. These objects know how to
read/write themselves from the stream, when asked to do so by the
VBentoNode objects that contain them.
*/
class VBentoAttribute
    {
    public:
    
        VBentoAttribute(); ///< Constructs with unitinitialized name.
        VBentoAttribute(VBinaryIOStream& stream, const VString& dataType); ///< Constructs by reading from stream.
        VBentoAttribute(const VString& name, const VString& dataType); ///< Constructs with name and type. @param name the attribute name @param dataType the data type
        virtual ~VBentoAttribute(); ///< Destructor.
        
        const VString& getName() const; ///< Returns the attribute name. @return a reference to the attribute name string.
        const VString& getDataType() const; ///< Returns the data type name. @return a reference to the data type name string.
        virtual void getValueAsString(VString& s) const = 0; ///< Returns a printable form of the attribute value.

        Vs64 calculateContentSize() const; ///< Returns the size, in bytes, of the attribute content if written to a binary stream. @return the attribute's binary size
        Vs64 calculateTotalSize() const; ///< Returns the size, in bytes, of the attribute content plus dynamic size indicator if written to a binary stream. @return the attribute's binary size
        void writeToStream(VBinaryIOStream& stream) const; ///< Writes the attribute to a binary stream. @param stream the stream to write to
        void writeToStream(VTextIOStream& stream) const; ///< Writes the attribute to a text stream as XML. @param stream the stream to write to
        void printStreamLayout(VString& info, int indentLevel) const; ///< Debugging method. Creates a text representation of the node's binary stream layout.
        void printStreamLayout(VHex& hexDump) const; ///< Debugging method. Prints a hex dump of the stream. @param hexDump the hex dump formatter object
        
        static VBentoAttribute* newObjectFromStream(VBinaryIOStream& stream); ///< Creates a new attribute object by reading a binary stream. @param stream the stream to read from @return the new object
        static VBentoAttribute* newObjectFromStream(VTextIOStream& stream); ///< Creates a new attribute object by reading a text XML stream. @param stream the stream to read from @return the new object

    protected:

        virtual Vs64 getDataLength() const = 0; ///< Returns the length of this object's raw data only; pure virtual. @return the length of the object's raw data
        virtual void writeDataToStream(VBinaryIOStream& stream) const = 0; ///< Writes the object's raw data only to a binary stream; pure virtual. @param stream the stream to write to
        virtual void writeDataToStream(VTextIOStream& stream) const = 0; ///< Writes the object's raw data only to a text stream as XML; pure virtual. @param stream the stream to write to
    
    private:
    
        VString mName;      ///< The attribute name.
        VString mDataType;  ///< The data type name.
    };
    
/**
VBentoS8 is a VBentoAttribute that holds a Vs8 value.
*/
class VBentoS8 : public VBentoAttribute
    {
    public:
    
        static const char* ID() { return "vs_8"; } ///< The data type name / class ID string.
    
        VBentoS8() {} ///< Constructs with unitinitialized name and value.
        VBentoS8(VBinaryIOStream& stream) : VBentoAttribute(stream, ID()), mValue(stream.readS8()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoS8(const VString& name, Vs8 i) : VBentoAttribute(name, ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoS8() {} ///< Destructor.

        virtual void getValueAsString(VString& s) const { s.format("%d 0x%02X", mValue, mValue); }
        
        inline Vs8 getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(Vs8 i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value
        
    protected:

        virtual Vs64 getDataLength() const { return 1; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToStream(VBinaryIOStream& stream) const { stream.writeS8(mValue); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void writeDataToStream(VTextIOStream& stream) const { VString s("%d", (int) mValue); stream.writeString(s); } ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to
    
    private:

        Vs8 mValue; ///< The attribute value.
    };

/**
VBentoU8 is a VBentoAttribute that holds a Vu8 value.
*/
class VBentoU8 : public VBentoAttribute
    {
    public:
    
        static const char* ID() { return "vu_8"; } ///< The data type name / class ID string.
    
        VBentoU8() {} ///< Constructs with unitinitialized name and value.
        VBentoU8(VBinaryIOStream& stream) : VBentoAttribute(stream, ID()), mValue(stream.readU8()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoU8(const VString& name, Vu8 i) : VBentoAttribute(name, ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoU8() {} ///< Destructor.
        
        virtual void getValueAsString(VString& s) const { s.format("%u 0x%02X", mValue, mValue); }
        
        inline Vu8 getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(Vu8 i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value
        
    protected:
        
        virtual Vs64 getDataLength() const { return 1; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToStream(VBinaryIOStream& stream) const { stream.writeU8(mValue); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void writeDataToStream(VTextIOStream& stream) const { VString s("%d", (int) mValue); stream.writeString(s); } ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to
    
    private:

        Vu8 mValue; ///< The attribute value.
    };

/**
VBentoS16 is a VBentoAttribute that holds a Vs16 value.
*/
class VBentoS16 : public VBentoAttribute
    {
    public:
    
        static const char* ID() { return "vs16"; } ///< The data type name / class ID string.
    
        VBentoS16() {} ///< Constructs with unitinitialized name and value.
        VBentoS16(VBinaryIOStream& stream) : VBentoAttribute(stream, ID()), mValue(stream.readS16()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoS16(const VString& name, Vs16 i) : VBentoAttribute(name, ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoS16() {} ///< Destructor.
        
        virtual void getValueAsString(VString& s) const { s.format("%hd 0x%04X", mValue, mValue); }
        
        inline Vs16 getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(Vs16 i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value
        
    protected:
        
        virtual Vs64 getDataLength() const { return 2; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToStream(VBinaryIOStream& stream) const { stream.writeS16(mValue); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void writeDataToStream(VTextIOStream& stream) const { VString s("%d", (int) mValue); stream.writeString(s); } ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to
    
    private:

        Vs16 mValue; ///< The attribute value.
    };

/**
VBentoU16 is a VBentoAttribute that holds a Vu16 value.
*/
class VBentoU16 : public VBentoAttribute
    {
    public:
    
        static const char* ID() { return "vu16"; } ///< The data type name / class ID string.
    
        VBentoU16() {} ///< Constructs with unitinitialized name and value.
        VBentoU16(VBinaryIOStream& stream) : VBentoAttribute(stream, ID()), mValue(stream.readU16()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoU16(const VString& name, Vu16 i) : VBentoAttribute(name, ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoU16() {} ///< Destructor.
        
        virtual void getValueAsString(VString& s) const { s.format("%hu 0x%04X", mValue, mValue); }
        
        inline Vu16 getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(Vu16 i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value
        
    protected:
        
        virtual Vs64 getDataLength() const { return 2; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToStream(VBinaryIOStream& stream) const { stream.writeU16(mValue); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void writeDataToStream(VTextIOStream& stream) const { VString s("%d", (int) mValue); stream.writeString(s); } ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to
    
    private:

        Vu16 mValue; ///< The attribute value.
    };

/**
VBentoS32 is a VBentoAttribute that holds a Vs32 value.
*/
class VBentoS32 : public VBentoAttribute
    {
    public:
    
        static const char* ID() { return "vs32"; } ///< The data type name / class ID string.
    
        VBentoS32() {} ///< Constructs with unitinitialized name and value.
        VBentoS32(VBinaryIOStream& stream) : VBentoAttribute(stream, ID()), mValue(stream.readS32()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoS32(const VString& name, Vs32 i) : VBentoAttribute(name, ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoS32() {} ///< Destructor.
        
        virtual void getValueAsString(VString& s) const { s.format("%ld 0x%08X", mValue, mValue); }
        
        inline Vs32 getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(Vs32 i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value
        
    protected:
        
        virtual Vs64 getDataLength() const { return 4; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToStream(VBinaryIOStream& stream) const { stream.writeS32(mValue); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void writeDataToStream(VTextIOStream& stream) const { VString s("%d", mValue); stream.writeString(s); } ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to
    
    private:

        Vs32 mValue; ///< The attribute value.
    };

/**
VBentoU32 is a VBentoAttribute that holds a Vu32 value.
*/
class VBentoU32 : public VBentoAttribute
    {
    public:
    
        static const char* ID() { return "vu32"; } ///< The data type name / class ID string.
    
        VBentoU32() {} ///< Constructs with unitinitialized name and value.
        VBentoU32(VBinaryIOStream& stream) : VBentoAttribute(stream, ID()), mValue(stream.readU32()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoU32(const VString& name, Vu32 i) : VBentoAttribute(name, ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoU32() {} ///< Destructor.
        
        virtual void getValueAsString(VString& s) const { s.format("%lu 0x%08X", mValue, mValue); }
        
        inline Vu32 getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(Vu32 i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value
        
    protected:
        
        virtual Vs64 getDataLength() const { return 4; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToStream(VBinaryIOStream& stream) const { stream.writeU32(mValue); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void writeDataToStream(VTextIOStream& stream) const { VString s("%u", mValue); stream.writeString(s); } ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to
    
    private:

        Vu32 mValue; ///< The attribute value.
    };

/**
VBentoS64 is a VBentoAttribute that holds a Vs64 value.
*/
class VBentoS64 : public VBentoAttribute
    {
    public:
    
        static const char* ID() { return "vs64"; } ///< The data type name / class ID string.
    
        VBentoS64() {} ///< Constructs with unitinitialized name and value.
        VBentoS64(VBinaryIOStream& stream) : VBentoAttribute(stream, ID()), mValue(stream.readS64()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoS64(const VString& name, Vs64 i) : VBentoAttribute(name, ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoS64() {} ///< Destructor.
        
        virtual void getValueAsString(VString& s) const { s.format("%lld 0x%016llX", mValue, mValue); }
        
        inline Vs64 getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(Vs64 i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value
        
    protected:
        
        virtual Vs64 getDataLength() const { return 8; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToStream(VBinaryIOStream& stream) const { stream.writeS64(mValue); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void writeDataToStream(VTextIOStream& stream) const { VString s("%lld", mValue); stream.writeString(s); } ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to
    
    private:

        Vs64 mValue; ///< The attribute value.
    };

/**
VBentoU64 is a VBentoAttribute that holds a Vu64 value.
*/
class VBentoU64 : public VBentoAttribute
    {
    public:
    
        static const char* ID() { return "vu64"; } ///< The data type name / class ID string.
    
        VBentoU64() {} ///< Constructs with unitinitialized name and value.
        VBentoU64(VBinaryIOStream& stream) : VBentoAttribute(stream, ID()), mValue(stream.readU64()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoU64(const VString& name, Vu64 i) : VBentoAttribute(name, ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoU64() {} ///< Destructor.
        
        virtual void getValueAsString(VString& s) const { s.format("%llu 0x%016llX", mValue, mValue); }
        
        inline Vu64 getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(Vu64 i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value
        
    protected:
        
        virtual Vs64 getDataLength() const { return 8; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToStream(VBinaryIOStream& stream) const { stream.writeU64(mValue); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void writeDataToStream(VTextIOStream& stream) const { VString s("%llu", mValue); stream.writeString(s); } ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to
    
    private:

        Vu64 mValue; ///< The attribute value.
    };

/**
VBentoBool is a VBentoAttribute that holds a bool value.
*/
class VBentoBool : public VBentoAttribute
    {
    public:
    
        static const char* ID() { return "bool"; } ///< The data type name / class ID string.
    
        VBentoBool() {} ///< Constructs with unitinitialized name and value.
        VBentoBool(VBinaryIOStream& stream) : VBentoAttribute(stream, ID()), mValue(stream.readBool()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoBool(const VString& name, bool b) : VBentoAttribute(name, ID()), mValue(b) {} ///< Constructs from supplied name and value.
        virtual ~VBentoBool() {} ///< Destructor.
        
        virtual void getValueAsString(VString& s) const { s.format("%s 0x%02X", (mValue ? "true":"false"), static_cast<Vu8>(mValue)); }
        
        inline bool getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(bool b) { mValue = b; } ///< Sets the attribute's value. @param b the attribute value
        
    protected:
        
        virtual Vs64 getDataLength() const { return 1; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToStream(VBinaryIOStream& stream) const { stream.writeBool(mValue); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void writeDataToStream(VTextIOStream& stream) const { stream.writeString(mValue?"true":"false"); } ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to
    
    private:

        bool mValue; ///< The attribute value.
    };

/**
VBentoString is a VBentoAttribute that holds a VString value.
*/
class VBentoString : public VBentoAttribute
    {
    public:
    
        static const char* ID() { return "vstr"; } ///< The data type name / class ID string.
    
        VBentoString() {} ///< Constructs with unitinitialized name and empty string.
        VBentoString(VBinaryIOStream& stream) : VBentoAttribute(stream, ID()) { stream.readString(mValue); } ///< Constructs by reading from stream. @param stream the stream to read
        VBentoString(const VString& name, const VString& s) : VBentoAttribute(name, ID()), mValue(s) {} ///< Constructs from supplied name and value.
        virtual ~VBentoString() {} ///< Destructor.
        
        virtual void getValueAsString(VString& s) const { s.format("\"%s\"", mValue.chars()); }
        
        inline const VString& getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the value string
        inline void setValue(const VString& s) { mValue = s; } ///< Sets the attribute's value. @param s the attribute value
        
    protected:
        
        virtual Vs64 getDataLength() const { return VBentoNode::_getBinaryStringLength(mValue); } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToStream(VBinaryIOStream& stream) const { stream.writeString(mValue); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void writeDataToStream(VTextIOStream& stream) const { stream.writeString(mValue); } ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to
    
    private:

        VString mValue; ///< The attribute value.
    };

/**
VBentoUnknownValue is a VBentoAttribute that holds a value that is read from
an input stream but whose type is unknown; the object uses a VMemoryStream
to hold the binary data of unknown type. The data's length is known. Its data
type name is known but the VBento code does not know how to map that data
type name to a C++ class, and must therefore use a VBentoUnknownValue.
*/
class VBentoUnknownValue : public VBentoAttribute
    {
    public:
    
        static const char* ID() { return "unkn"; } ///< The data type name / class ID string.
    
        VBentoUnknownValue() {} ///< Constructs with unitinitialized name and empty stream.
        VBentoUnknownValue(VBinaryIOStream& stream, Vs64 dataLength, const VString& dataType); ///< Constructs by reading from stream. @param stream the stream to read @param dataLength the length of stream data to read @param dataType the original data type value
        virtual ~VBentoUnknownValue() {} ///< Destructor.
        
        virtual void getValueAsString(VString& s) const { VHex::bufferToHexString(mValue.getBuffer(), mValue.eofOffset(), s, true/* want leading "0x" */); }
        
        inline const VMemoryStream& getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the unknown-typed data stream
        
    protected:
        
        virtual Vs64 getDataLength() const { return mValue.eofOffset(); } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToStream(VBinaryIOStream& stream) const; ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void writeDataToStream(VTextIOStream& stream) const { stream.writeString("(binary data)"); } ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to
    
    private:

        VMemoryStream mValue; ///< The attribute value.
    };

static void _printIndent(VString& output, int indentLevel)
    {
    for (int i = 0; i < indentLevel; ++i)
        output += "  ";
    }

// VBentoAttribute -----------------------------------------------------------

VBentoAttribute::VBentoAttribute()
: mName("uninitialized"), mDataType(VString::EMPTY())
    {
    }

VBentoAttribute::VBentoAttribute(VBinaryIOStream& stream, const VString& dataType)
: mName(VString::EMPTY()), mDataType(dataType)
    {
    stream.readString(mName);
    }

VBentoAttribute::VBentoAttribute(const VString& name, const VString& dataType)
: mName(name), mDataType(dataType)
    {
    }

VBentoAttribute::~VBentoAttribute()
    {
    }

const VString& VBentoAttribute::getName() const
    {
    return mName;
    }

const VString& VBentoAttribute::getDataType() const
    {
    return mDataType;
    }

Vs64 VBentoAttribute::calculateContentSize() const
    {
    Vs64    lengthOfType = 4;
    Vs64    lengthOfName = VBentoNode::_getBinaryStringLength(mName);
    Vs64    lengthOfData = this->getDataLength();

    return lengthOfType + lengthOfName + lengthOfData;
    }

Vs64 VBentoAttribute::calculateTotalSize() const
    {
    Vs64    contentSize = this->calculateContentSize();
    Vs64    lengthOfLength = VBentoNode::_getLengthOfLength(contentSize);

    return lengthOfLength + contentSize;
    }

void VBentoAttribute::writeToStream(VBinaryIOStream& stream) const
    {
    Vs64    contentSize = this->calculateContentSize();

    VBentoNode::_writeLengthToStream(stream, contentSize);
    VBentoNode::_writeFourCharCodeToStream(stream, mDataType);
    stream.writeString(mName);
    
    this->writeDataToStream(stream);
    }

void VBentoAttribute::writeToStream(VTextIOStream& stream) const
    {
    VString    equalChar('=');
    VString    colonChar(':');
    VString    quote('\"');
    
    stream.writeString(mName);
    stream.writeString(colonChar);
    stream.writeString(mDataType);
    stream.writeString(equalChar);
    stream.writeString(quote);

    this->writeDataToStream(stream);

    stream.writeString(quote);
    }

void VBentoAttribute::printStreamLayout(VString& info, int indentLevel) const
    {
    VString valueString;
    this->getValueAsString(valueString);
    
    _printIndent(info, indentLevel);
    info += "attribute '";
    info += mName;
    info += "' (";
    info += mDataType;
    info += ") = ";
    info += valueString;
    info += '\n';
    }

void VBentoAttribute::printStreamLayout(VHex& hexDump) const
    {
    Vs64    totalSize = this->calculateTotalSize();
    std::cout << "Attribute '" << mName << "': length= " << totalSize << ", type=" << mDataType << std::endl;

    VMemoryStream    buffer;
    VBinaryIOStream    stream(buffer);
    
    this->writeToStream(stream);

    hexDump.printHex(buffer.getBuffer(), buffer.eofOffset());
    }

VBentoAttribute* VBentoAttribute::newObjectFromStream(VBinaryIOStream& stream)
    {
    Vs64    theDataLength = VBentoNode::_readLengthFromStream(stream);
    VString    theDataType;

    VBentoNode::_readFourCharCodeFromStream(stream, theDataType);
    
    if (theDataType == VBentoS8::ID())
        return new VBentoS8(stream);
    else if (theDataType == VBentoU8::ID())
        return new VBentoU8(stream);
    else if (theDataType == VBentoS16::ID())
        return new VBentoS16(stream);
    else if (theDataType == VBentoU16::ID())
        return new VBentoU16(stream);
    else if (theDataType == VBentoS32::ID())
        return new VBentoS32(stream);
    else if (theDataType == VBentoU32::ID())
        return new VBentoU32(stream);
    else if (theDataType == VBentoS64::ID())
        return new VBentoS64(stream);
    else if (theDataType == VBentoU64::ID())
        return new VBentoU64(stream);
    else if (theDataType == VBentoBool::ID())
        return new VBentoBool(stream);
    else if (theDataType == VBentoString::ID())
        return new VBentoString(stream);
    else
        return new VBentoUnknownValue(stream, theDataLength, theDataType);
    }

VBentoAttribute* VBentoAttribute::newObjectFromStream(VTextIOStream& /*stream*/)
    {
    // Reading unknown data types from a text stream is not (yet) supported.
    return new VBentoUnknownValue();
    }

// VBentoNode ----------------------------------------------------------------

VBentoNode::VBentoNode()
: mName("uninitialized")
    {
    }

VBentoNode::VBentoNode(const VString& name)
: mName(name)
    {
    }

VBentoNode::VBentoNode(VBinaryIOStream& stream)
    {
    this->readFromStream(stream);
    }

VBentoNode::~VBentoNode()
    {
    VSizeType    numAttributes = mAttributes.size();
    for (VSizeType i = 0; i < numAttributes; ++i)
        delete mAttributes[i];
    
    VSizeType    numChildNodes = mChildNodes.size();
    for (VSizeType i = 0; i < numChildNodes; ++i)
        delete mChildNodes[i];
    }

void VBentoNode::addChildNode(VBentoNode* node)
    {
    mChildNodes.push_back(node);
    }

VBentoNode* VBentoNode::addNewChildNode(const VString& name)
    {
    VBentoNode* child = new VBentoNode(name);
    mChildNodes.push_back(child);
    return child;
    }

void VBentoNode::addS8(const VString& name, Vs8 value) { this->_addAttribute(new VBentoS8(name, value)); }
void VBentoNode::addU8(const VString& name, Vu8 value) { this->_addAttribute(new VBentoU8(name, value)); }
void VBentoNode::addS16(const VString& name, Vs16 value) { this->_addAttribute(new VBentoS16(name, value)); }
void VBentoNode::addU16(const VString& name, Vu16 value) { this->_addAttribute(new VBentoU16(name, value)); }
void VBentoNode::addS32(const VString& name, Vs32 value) { this->_addAttribute(new VBentoS32(name, value)); }
void VBentoNode::addU32(const VString& name, Vu32 value) { this->_addAttribute(new VBentoU32(name, value)); }
void VBentoNode::addS64(const VString& name, Vs64 value) { this->_addAttribute(new VBentoS64(name, value)); }
void VBentoNode::addU64(const VString& name, Vu64 value) { this->_addAttribute(new VBentoU64(name, value)); }
void VBentoNode::addBool(const VString& name, bool value) { this->_addAttribute(new VBentoBool(name, value)); }
void VBentoNode::addString(const VString& name, const VString& value) { this->_addAttribute(new VBentoString(name, value)); }
void VBentoNode::addInt(const VString& name, int value) { this->addS32(name, static_cast<Vs32>(value)); }

void VBentoNode::writeToStream(VBinaryIOStream& stream) const
    {
    Vs64    contentSize = this->_calculateContentSize();
    VBentoNode::_writeLengthToStream(stream, contentSize);

    VSizeType    numAttributes = mAttributes.size();
    stream.writeS32(static_cast<Vs32> (numAttributes));

    VSizeType    numChildNodes = mChildNodes.size();
    stream.writeS32(static_cast<Vs32> (numChildNodes));

    stream.writeString(mName);

    for (VSizeType i = 0; i < numAttributes; ++i)
        mAttributes[i]->writeToStream(stream);
    
    for (VSizeType i = 0; i < numChildNodes; ++i)
        mChildNodes[i]->writeToStream(stream);
    }

void VBentoNode::readFromStream(VBinaryIOStream& stream)
    {
    /* unused Vs64    contentSize = */ (void) VBentoNode::_readLengthFromStream(stream);
    Vs32    numAttributes = stream.readS32();
    Vs32    numChildNodes = stream.readS32();

    stream.readString(mName);

    for (int i = 0; i < numAttributes; ++i)
        this->_addAttribute(VBentoAttribute::newObjectFromStream(stream));

    for (int i = 0; i < numChildNodes; ++i)
        this->addChildNode(new VBentoNode(stream));
    }

const VBentoNodePtrVector& VBentoNode::getNodes() const
    {
    return mChildNodes;
    }

const VBentoNode* VBentoNode::findNode(const VString& nodeName) const
    {
    for (VBentoNodePtrVector::const_iterator i = mChildNodes.begin(); i != mChildNodes.end(); ++i)
        {
        if ((*i)->getName() == nodeName)
            return (*i);
        }

    return NULL;
    }
    
const VBentoNode* VBentoNode::findNode(const VString& nodeName, const VString& attributeName, const VString& dataType) const
    {
    for (VBentoNodePtrVector::const_iterator i = mChildNodes.begin(); i != mChildNodes.end(); ++i)
        {
        if ((*i)->getName() == nodeName)
            {
            if ((*i)->_findAttribute(attributeName, dataType) != NULL)
                return (*i);
            }
        }
    
    return NULL;
    }

Vs8 VBentoNode::getS8(const VString& name, Vs8 defaultValue) const
    {
    const VBentoS8*    attribute = dynamic_cast<const VBentoS8*> (this->_findAttribute(name, VBentoS8::ID()));
    
    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

Vs8 VBentoNode::getS8(const VString& name) const
    {
    const VBentoS8*    attribute = dynamic_cast<const VBentoS8*> (this->_findAttribute(name, VBentoS8::ID()));
    
    if (attribute == NULL)
        throw VException("Attribute %s not found.", name.chars());
    else
        return attribute->getValue();
    }

Vu8 VBentoNode::getU8(const VString& name, Vu8 defaultValue) const
    {
    const VBentoU8*    attribute = dynamic_cast<const VBentoU8*> (this->_findAttribute(name, VBentoU8::ID()));
    
    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

Vu8 VBentoNode::getU8(const VString& name) const
    {
    const VBentoU8*    attribute = dynamic_cast<const VBentoU8*> (this->_findAttribute(name, VBentoU8::ID()));
    
    if (attribute == NULL)
        throw VException("Attribute %s not found.", name.chars());
    else
        return attribute->getValue();
    }

Vs16 VBentoNode::getS16(const VString& name, Vs16 defaultValue) const
    {
    const VBentoS16*    attribute = dynamic_cast<const VBentoS16*> (this->_findAttribute(name, VBentoS16::ID()));
    
    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

Vs16 VBentoNode::getS16(const VString& name) const
    {
    const VBentoS16*    attribute = dynamic_cast<const VBentoS16*> (this->_findAttribute(name, VBentoS16::ID()));
    
    if (attribute == NULL)
        throw VException("Attribute %s not found.", name.chars());
    else
        return attribute->getValue();
    }

Vu16 VBentoNode::getU16(const VString& name, Vu16 defaultValue) const
    {
    const VBentoU16*    attribute = dynamic_cast<const VBentoU16*> (this->_findAttribute(name, VBentoU16::ID()));
    
    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

Vu16 VBentoNode::getU16(const VString& name) const
    {
    const VBentoU16*    attribute = dynamic_cast<const VBentoU16*> (this->_findAttribute(name, VBentoU16::ID()));
    
    if (attribute == NULL)
        throw VException("Attribute %s not found.", name.chars());
    else
        return attribute->getValue();
    }

Vs32 VBentoNode::getS32(const VString& name, Vs32 defaultValue) const
    {
    const VBentoS32*    attribute = dynamic_cast<const VBentoS32*> (this->_findAttribute(name, VBentoS32::ID()));
    
    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

Vs32 VBentoNode::getS32(const VString& name) const
    {
    const VBentoS32*    attribute = dynamic_cast<const VBentoS32*> (this->_findAttribute(name, VBentoS32::ID()));
    
    if (attribute == NULL)
        throw VException("Attribute %s not found.", name.chars());
    else
        return attribute->getValue();
    }

Vu32 VBentoNode::getU32(const VString& name, Vu32 defaultValue) const
    {
    const VBentoU32*    attribute = dynamic_cast<const VBentoU32*> (this->_findAttribute(name, VBentoU32::ID()));
    
    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

Vu32 VBentoNode::getU32(const VString& name) const
    {
    const VBentoU32*    attribute = dynamic_cast<const VBentoU32*> (this->_findAttribute(name, VBentoU32::ID()));
    
    if (attribute == NULL)
        throw VException("Attribute %s not found.", name.chars());
    else
        return attribute->getValue();
    }

Vs64 VBentoNode::getS64(const VString& name, Vs64 defaultValue) const
    {
    const VBentoS64*    attribute = dynamic_cast<const VBentoS64*> (this->_findAttribute(name, VBentoS64::ID()));
    
    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

Vs64 VBentoNode::getS64(const VString& name) const
    {
    const VBentoS64*    attribute = dynamic_cast<const VBentoS64*> (this->_findAttribute(name, VBentoS64::ID()));
    
    if (attribute == NULL)
        throw VException("Attribute %s not found.", name.chars());
    else
        return attribute->getValue();
    }

Vu64 VBentoNode::getU64(const VString& name, Vu64 defaultValue) const
    {
    const VBentoU64*    attribute = dynamic_cast<const VBentoU64*> (this->_findAttribute(name, VBentoU64::ID()));
    
    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

Vu64 VBentoNode::getU64(const VString& name) const
    {
    const VBentoU64*    attribute = dynamic_cast<const VBentoU64*> (this->_findAttribute(name, VBentoU64::ID()));
    
    if (attribute == NULL)
        throw VException("Attribute %s not found.", name.chars());
    else
        return attribute->getValue();
    }

bool VBentoNode::getBool(const VString& name, bool defaultValue) const
    {
    const VBentoBool*    attribute = dynamic_cast<const VBentoBool*> (this->_findAttribute(name, VBentoBool::ID()));
    
    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

bool VBentoNode::getBool(const VString& name) const
    {
    const VBentoBool*    attribute = dynamic_cast<const VBentoBool*> (this->_findAttribute(name, VBentoBool::ID()));
    
    if (attribute == NULL)
        throw VException("Attribute %s not found.", name.chars());
    else
        return attribute->getValue();
    }

const VString& VBentoNode::getString(const VString& name, const VString& defaultValue) const
    {
    const VBentoString*    attribute = dynamic_cast<const VBentoString*> (this->_findAttribute(name, VBentoString::ID()));
    
    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->getValue();
    }

const VString& VBentoNode::getString(const VString& name) const
    {
    const VBentoString*    attribute = dynamic_cast<const VBentoString*> (this->_findAttribute(name, VBentoString::ID()));
    
    if (attribute == NULL)
        throw VException("Attribute %s not found.", name.chars());
    else
        return attribute->getValue();
    }

int VBentoNode::getInt(const VString& name, int defaultValue) const
    {
    return static_cast<int>(this->getS32(name, static_cast<Vs32>(defaultValue)));
    }

int VBentoNode::getInt(const VString& name) const
    {
    return static_cast<int>(this->getS32(name));
    }

const VString& VBentoNode::getName() const
    {
    return mName;
    }

void VBentoNode::writeToStream(VTextIOStream& stream, int indentLevel) const
    {
    VSizeType    numAttributes = mAttributes.size();
    VSizeType    numChildNodes = mChildNodes.size();
    
    VString    s;
    
    for (int i = 0; i < indentLevel; ++i)
        s += ' ';

    s += "<";
    s += mName;
    
    stream.writeString(s);

    s = ' ';
    for (VSizeType i = 0; i < numAttributes; ++i)
        {
        stream.writeString(s);    // the space before this attribute
        mAttributes[i]->writeToStream(stream);
        }

    if (numChildNodes == 0)
        s = " />";
    else
        s = '>';
        
    stream.writeLine(s);
    
    for (VSizeType i = 0; i < numChildNodes; ++i)
        mChildNodes[i]->writeToStream(stream, indentLevel + 1);

    if (numChildNodes != 0)
        {
        s = VString::EMPTY();

        for (int i = 0; i < indentLevel; ++i)
            s += ' ';

        s += '<';
        s += '/';
        s += mName;
        s += '>';

        stream.writeLine(s);
        }
    }

void VBentoNode::printXML() const
    {
    try
        {
        VBufferedFileStream    stdoutStream(stdout, false/*don't close on destruct*/);
        VTextIOStream        printStream(stdoutStream, VTextIOStream::kUseUnixLineEndings);
        
        this->writeToStream(printStream);
        
        stdoutStream.flush();
        }
    catch (const VException& ex)
        {
        std::cout << "VBentoNode::printXML unable to print: '" << ex.what() << "'" << std::endl;
        }
    }

void VBentoNode::getStreamLayoutText(VString& info) const
    {
    this->printStreamLayout(info, 0);
    }

void VBentoNode::printStreamLayoutText() const
    {
    VString info;
    this->printStreamLayout(info, 0);
    std::cout << info << std::endl;
    }

void VBentoNode::printStreamLayout(VString& info, int indentLevel) const
    {
    _printIndent(info, indentLevel);
    info += "node '";
    info += mName;
    info += "'";
    info += '\n';
    
    for (VBentoAttributePtrVector::const_iterator i = mAttributes.begin(); i != mAttributes.end(); ++i)
        (*i)->printStreamLayout(info, indentLevel + 1);
    
    for (VBentoNodePtrVector::const_iterator i = mChildNodes.begin(); i != mChildNodes.end(); ++i)
        (*i)->printStreamLayout(info, indentLevel + 1);
    }

void VBentoNode::printStreamLayout(VHex& hexDump) const
    {
    VMemoryStream    buffer;
    VBinaryIOStream    stream(buffer);

    VSizeType    numAttributes = mAttributes.size();
    VSizeType    numChildNodes = mChildNodes.size();

    Vs64    totalSize = this->_calculateContentSize();
    
    VBentoNode::_writeLengthToStream(stream, totalSize);
    stream.writeS32(static_cast<Vs32> (numAttributes));
    stream.writeS32(static_cast<Vs32> (numChildNodes));
    stream.writeString(mName);

    // Regarding casting to (int) here: otherwise, MSVC++7 warns of conversion to unsigned int for the VSizeType variables.
    std::cout << std::endl << "Node '" << mName << "': length=" << totalSize << ", numAttributes=" << (int) numAttributes << ", numChildNodes=" << (int) numChildNodes << std::endl;

    hexDump.printHex(buffer.getBuffer(), buffer.eofOffset());

    if (numAttributes > 0)
        std::cout << std::endl << "Node '" << mName << "' attributes" << std::endl;

    for (VSizeType i = 0; i < numAttributes; ++i)
        mAttributes[i]->printStreamLayout(hexDump);
    
    if (numChildNodes > 0)
        std::cout << std::endl << "Node '" << mName << "' child nodes" << std::endl;

    for (VSizeType i = 0; i < numChildNodes; ++i)
        mChildNodes[i]->printStreamLayout(hexDump);
    }

Vs64 VBentoNode::_calculateContentSize() const
    {
    Vs64 lengthOfCounters = 8; // 4 bytes each for #attributes and #children
    Vs64 lengthOfName = VBentoNode::_getBinaryStringLength(mName);

    Vs64 lengthOfAttributes = 0;
    for (VBentoAttributePtrVector::const_iterator i = mAttributes.begin(); i != mAttributes.end(); ++i)
        lengthOfAttributes += (*i)->calculateTotalSize();
    
    Vs64 lengthOfChildren = 0;
    for (VBentoNodePtrVector::const_iterator i = mChildNodes.begin(); i != mChildNodes.end(); ++i)
        lengthOfChildren += (*i)->_calculateTotalSize();

    Vs64 contentSize = lengthOfCounters + lengthOfName + lengthOfAttributes + lengthOfChildren;
    
    return contentSize;
    }

Vs64 VBentoNode::_calculateTotalSize() const
    {
    Vs64    contentSize = this->_calculateContentSize();
    Vs64    lengthOfLength = VBentoNode::_getLengthOfLength(contentSize);

    return lengthOfLength + contentSize;
    }

void VBentoNode::_addAttribute(VBentoAttribute* attribute)
    {
    mAttributes.push_back(attribute);
    }

const VBentoAttributePtrVector& VBentoNode::_getAttributes() const
    {
    return mAttributes;
    }

const VBentoAttribute* VBentoNode::_findAttribute(const VString& name, const VString& dataType) const
    {
    for (VBentoAttributePtrVector::const_iterator i = mAttributes.begin(); i != mAttributes.end(); ++i)
        {
        if (((*i)->getName() == name) &&
            ((*i)->getDataType() == dataType))
            return (*i);
        }
    
    return NULL;
    }

// static
Vs64 VBentoNode::_readLengthFromStream(VBinaryIOStream& stream)
    {
    return stream.readDynamicCount();
    }

// static
void VBentoNode::_writeLengthToStream(VBinaryIOStream& stream, Vs64 length)
    {
    stream.writeDynamicCount(length);
    }

// static
Vs64 VBentoNode::_getLengthOfLength(Vs64 length)
    {
    return VBinaryIOStream::getDynamicCountLength(length);
    }

// static
void VBentoNode::_readFourCharCodeFromStream(VBinaryIOStream& stream, VString& code)
    {
    code.preflight(4);
    (void) stream.read(reinterpret_cast<Vu8*> (code.buffer()), CONST_S64(4));
    code.postflight(4);
    }

// static
void VBentoNode::_writeFourCharCodeToStream(VBinaryIOStream& stream, const VString& code)
    {
    int    codeLength = code.length();

    (void) stream.write(reinterpret_cast<Vu8*> (code.chars()), V_MIN(4, codeLength));
    
    // In case code is less than 4 chars, pad with spaces. Please don't use such codes,
    // it's not efficient!
    for (int i = codeLength; i < 4; ++i)
        stream.writeS8(' ');
    }

// static
Vs64 VBentoNode::_getBinaryStringLength(const VString& s)
    {
    int textLength = s.length();
    Vs64 lengthOfLength = VBentoNode::_getLengthOfLength(textLength);

    return lengthOfLength + textLength;
    }

// VBentoUnknownValue --------------------------------------------------------

VBentoUnknownValue::VBentoUnknownValue(VBinaryIOStream& stream, Vs64 dataLength, const VString& dataType) :
VBentoAttribute(stream, dataType),
mValue(dataLength)
    {
    VBinaryIOStream    memoryIOStream(mValue);
    
    streamCopy(stream, memoryIOStream, dataLength);
    }

void VBentoUnknownValue::writeDataToStream(VBinaryIOStream& stream) const
    {
    // To ensure that there are no side-effects and we are indeed const in behavior,
    // we save and restore mValue stream's offset, while doing a const-cast so
    // that we are allowed to use manipulate the stream.
    Vs64 savedOffset = mValue.offset();
    VBinaryIOStream    memoryIOStream(const_cast<VBentoUnknownValue*>(this)->mValue);
    
    memoryIOStream.seek(0, SEEK_SET);
    streamCopy(memoryIOStream, stream, mValue.eofOffset());
    memoryIOStream.seek(savedOffset, SEEK_SET);
    }

// VBentoCallbackParser ------------------------------------------------------

VBentoCallbackParser::VBentoCallbackParser(VBinaryIOStream& stream)
    {
    VBentoCallbackParser::processNode(0, stream);
    }

void VBentoCallbackParser::processNode(int depth, VBinaryIOStream& stream)
    {
    Vs64    theDataLength = VBentoNode::_readLengthFromStream(stream);
    Vs32    numAttributes = stream.readS32();
    Vs32    numChildNodes = stream.readS32();
    VString    theName;

    stream.readString(theName);
    
    this->nodeHeaderComplete(depth, theDataLength, numAttributes, numChildNodes, theName);

    for (int i = 0; i < numAttributes; ++i)
        {
        this->processAttribute(depth, stream);
        }

    this->nodeAttributesComplete(depth, theDataLength, numAttributes, numChildNodes, theName);

    for (int i = 0; i < numChildNodes; ++i)
        {
        this->processNode(depth+1, stream);
        }

    this->nodeComplete(depth, theDataLength, numAttributes, numChildNodes, theName);
    }

void VBentoCallbackParser::processAttribute(int depth, VBinaryIOStream& stream)
    {
    Vs64    theDataLength = VBentoNode::_readLengthFromStream(stream);
    VString    type;
    VString    theName;

    VBentoNode::_readFourCharCodeFromStream(stream, type);
    stream.readString(theName);
    
    this->attributeHeaderComplete(depth, theDataLength, type, theName);

    this->readAttributeData(depth, stream, static_cast<Vu64> (theDataLength));

    this->attributeComplete(depth, theDataLength, type, theName);
    }

void VBentoCallbackParser::nodeHeaderComplete(int /*depth*/, Vs64 /*length*/, Vs32 /*numAttributes*/, Vs32 /*numChildren*/, const VString& /*name*/)
    {
    }

void VBentoCallbackParser::nodeAttributesComplete(int /*depth*/, Vs64 /*length*/, Vs32 /*numAttributes*/, Vs32 /*numChildren*/, const VString& /*name*/)
    {
    }

void VBentoCallbackParser::nodeComplete(int /*depth*/, Vs64 /*length*/, Vs32 /*numAttributes*/, Vs32 /*numChildren*/, const VString& /*name*/)
    {
    }

void VBentoCallbackParser::attributeHeaderComplete(int /*depth*/, Vs64 /*length*/, const VString& /*type*/, const VString& /*name*/)
    {
    }

void VBentoCallbackParser::attributeComplete(int /*depth*/, Vs64 /*length*/, const VString& /*type*/, const VString& /*name*/)
    {
    }

void VBentoCallbackParser::readAttributeData(int /*depth*/, VBinaryIOStream& stream, Vu64 dataLength)
    {
    stream.skip(static_cast<Vu64> (dataLength));
    }

