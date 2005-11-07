/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

#ifndef vbento_h
#define vbento_h

/** @file */

#include "vbinaryiostream.h"
#include "vtextiostream.h"
#include "vmemorystream.h"

class VHex;

/*
VBento
Bento presents an extensible, typed, named data hierarchy; it can be used as
an abstraction of an XML DOM or a binary version of the same. A large part of
its purpose is to allow you to write data (a wire protocol for example) such
that the reader can read the data, without any effort, with following two
important backward/forward-compatibility aspects:

- unexpected parameters are harmlessly ignored
- unspecified parameters can be set to defaults

This allows you to revise the protocol with new parameters, without breaking
the binary compatibility of old clients and old servers.

Bento binary stream format:
array of atoms
atom =
    Vs64 total length of atom
    Vs32 number of attributes
    Vs32 number of child atoms
    VString atom name
    array of attributes
    array of child atoms

attribute =
    Vs64 total length of attribute
    VString name
    VString type
    raw data
*/

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
    
        VBentoAttribute();    ///< Constructs with unitinitialized name.
        VBentoAttribute(VBinaryIOStream& stream, const VString& inDataType);    ///< Constructs by reading from stream.
        VBentoAttribute(const VString& inName, const VString& inDataType);    ///< Constructs with name and type. @param name the attribute name @param inDataType the data type
        virtual ~VBentoAttribute();    ///< Destructor.
        
        const VString&        name() const;        ///< Returns the attribute name. @return a reference to the attribute name string.
        const VString&        dataType() const;    ///< Returns the data type name. @return a reference to the data type name string.

        Vs64 calculateBinarySize() const;    ///< Returns the size, in bytes, of the attribute if written to a binary stream. @return the attribute's binary size
        void writeToStream(VBinaryIOStream& stream) const;    ///< Writes the attribute to a binary stream. @param stream the stream to write to
        void writeToStream(VTextIOStream& stream) const;    ///< Writes the attribute to a text stream as XML. @param stream the stream to write to
        void printStreamLayout(VHex& hexDump) const;    ///< Debugging method. Prints a hex dump of the stream. @param hexDump the hex dump formatter object
        
        static VBentoAttribute* newObjectFromStream(VBinaryIOStream& stream);    ///< Creates a new attribute object by reading a binary stream. @param stream the stream to read from @return the new object
        static VBentoAttribute* newObjectFromStream(VTextIOStream& stream);    ///< Creates a new attribute object by reading a text XML stream. @param stream the stream to read from @return the new object

    protected:

        virtual Vs64    dataLength() const = 0;    ///< Returns the length of this object's raw data only; pure virtual. @return the length of the object's raw data
        virtual void    writeDataToStream(VBinaryIOStream& stream) const = 0;    ///< Writes the object's raw data only to a binary stream; pure virtual. @param stream the stream to write to
        virtual void    writeDataToStream(VTextIOStream& stream) const = 0;    ///< Writes the object's raw data only to a text stream as XML; pure virtual. @param stream the stream to write to
        virtual const char*    classID() const = 0;    ///< Returns the object's class ID char string; pure virtual. @return the class ID string
    
    private:
    
        VString            mName;        ///< The attribute name.
        VString            mDataType;    ///< The data type name.
    };
    
typedef std::vector<VBentoAttribute*> VBentoAttributePtrVector;

/**
VBentoS8 is a VBentoAttribute that holds a Vs8 value.
*/
class VBentoS8 : public VBentoAttribute
    {
    public:
    
        static const char* id() { return "vs_8"; }    ///< The data type name / class ID string.
    
        VBentoS8() {}    ///< Constructs with unitinitialized name and value.
        VBentoS8(VBinaryIOStream& stream) : VBentoAttribute(stream, id()), mValue(stream.readS8()) {}    ///< Constructs by reading from stream. @param stream the stream to read
        VBentoS8(const VString& name, Vs8 i) : VBentoAttribute(name, id()), mValue(i) {}    ///< Constructs from supplied name and value.
        virtual ~VBentoS8() {}    ///< Destructor.
        
        inline Vs8    value() const { return mValue; }    ///< Returns the attribute's value. @return the value
        inline void    setValue(Vs8 i) { mValue = i; }    ///< Sets the attribute's value. @param i the attribute value
        
    protected:

        virtual Vs64    dataLength() const { return 1; }    ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void    writeDataToStream(VBinaryIOStream& stream) const { stream.writeS8(mValue); }    ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void    writeDataToStream(VTextIOStream& stream) const { VString s("%d", (int) mValue); stream.writeString(s); }    ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to
        virtual const char*    classID() const { return id(); }    ///< Returns the object's class ID char string. @return the class ID string
    
    private:

        Vs8    mValue;    ///< The attribute value.
    };

/**
VBentoU8 is a VBentoAttribute that holds a Vu8 value.
*/
class VBentoU8 : public VBentoAttribute
    {
    public:
    
        static const char* id() { return "vu_8"; }    ///< The data type name / class ID string.
    
        VBentoU8() {}    ///< Constructs with unitinitialized name and value.
        VBentoU8(VBinaryIOStream& stream) : VBentoAttribute(stream, id()), mValue(stream.readU8()) {}    ///< Constructs by reading from stream. @param stream the stream to read
        VBentoU8(const VString& name, Vu8 i) : VBentoAttribute(name, id()), mValue(i) {}    ///< Constructs from supplied name and value.
        virtual ~VBentoU8() {}    ///< Destructor.
        
        inline Vu8    value() const { return mValue; }    ///< Returns the attribute's value. @return the value
        inline void    setValue(Vu8 i) { mValue = i; }    ///< Sets the attribute's value. @param i the attribute value
        
    protected:
        
        virtual Vs64    dataLength() const { return 1; }    ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void    writeDataToStream(VBinaryIOStream& stream) const { stream.writeU8(mValue); }    ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void    writeDataToStream(VTextIOStream& stream) const { VString s("%d", (int) mValue); stream.writeString(s); }    ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to
        virtual const char*    classID() const { return id(); }    ///< Returns the object's class ID char string. @return the class ID string
    
    private:

        Vu8    mValue;    ///< The attribute value.
    };

/**
VBentoS16 is a VBentoAttribute that holds a Vs16 value.
*/
class VBentoS16 : public VBentoAttribute
    {
    public:
    
        static const char* id() { return "vs16"; }    ///< The data type name / class ID string.
    
        VBentoS16() {}    ///< Constructs with unitinitialized name and value.
        VBentoS16(VBinaryIOStream& stream) : VBentoAttribute(stream, id()), mValue(stream.readS16()) {}    ///< Constructs by reading from stream. @param stream the stream to read
        VBentoS16(const VString& name, Vs16 i) : VBentoAttribute(name, id()), mValue(i) {}    ///< Constructs from supplied name and value.
        virtual ~VBentoS16() {}    ///< Destructor.
        
        inline Vs16    value() const { return mValue; }    ///< Returns the attribute's value. @return the value
        inline void    setValue(Vs16 i) { mValue = i; }    ///< Sets the attribute's value. @param i the attribute value
        
    protected:
        
        virtual Vs64    dataLength() const { return 2; }    ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void    writeDataToStream(VBinaryIOStream& stream) const { stream.writeS16(mValue); }    ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void    writeDataToStream(VTextIOStream& stream) const { VString s("%d", (int) mValue); stream.writeString(s); }    ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to
        virtual const char*    classID() const { return id(); }    ///< Returns the object's class ID char string. @return the class ID string
    
    private:

        Vs16    mValue;    ///< The attribute value.
    };

/**
VBentoU16 is a VBentoAttribute that holds a Vu16 value.
*/
class VBentoU16 : public VBentoAttribute
    {
    public:
    
        static const char* id() { return "vu16"; }    ///< The data type name / class ID string.
    
        VBentoU16() {}    ///< Constructs with unitinitialized name and value.
        VBentoU16(VBinaryIOStream& stream) : VBentoAttribute(stream, id()), mValue(stream.readU16()) {}    ///< Constructs by reading from stream. @param stream the stream to read
        VBentoU16(const VString& name, Vu16 i) : VBentoAttribute(name, id()), mValue(i) {}    ///< Constructs from supplied name and value.
        virtual ~VBentoU16() {}    ///< Destructor.
        
        inline Vu16    value() const { return mValue; }    ///< Returns the attribute's value. @return the value
        inline void    setValue(Vu16 i) { mValue = i; }    ///< Sets the attribute's value. @param i the attribute value
        
    protected:
        
        virtual Vs64    dataLength() const { return 2; }    ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void    writeDataToStream(VBinaryIOStream& stream) const { stream.writeU16(mValue); }    ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void    writeDataToStream(VTextIOStream& stream) const { VString s("%d", (int) mValue); stream.writeString(s); }    ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to
        virtual const char*    classID() const { return id(); }    ///< Returns the object's class ID char string. @return the class ID string
    
    private:

        Vu16    mValue;    ///< The attribute value.
    };

/**
VBentoS32 is a VBentoAttribute that holds a Vs32 value.
*/
class VBentoS32 : public VBentoAttribute
    {
    public:
    
        static const char* id() { return "vs32"; }    ///< The data type name / class ID string.
    
        VBentoS32() {}    ///< Constructs with unitinitialized name and value.
        VBentoS32(VBinaryIOStream& stream) : VBentoAttribute(stream, id()), mValue(stream.readS32()) {}    ///< Constructs by reading from stream. @param stream the stream to read
        VBentoS32(const VString& name, Vs32 i) : VBentoAttribute(name, id()), mValue(i) {}    ///< Constructs from supplied name and value.
        virtual ~VBentoS32() {}    ///< Destructor.
        
        inline Vs32    value() const { return mValue; }    ///< Returns the attribute's value. @return the value
        inline void    setValue(Vs32 i) { mValue = i; }    ///< Sets the attribute's value. @param i the attribute value
        
    protected:
        
        virtual Vs64    dataLength() const { return 4; }    ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void    writeDataToStream(VBinaryIOStream& stream) const { stream.writeS32(mValue); }    ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void    writeDataToStream(VTextIOStream& stream) const { VString s("%d", mValue); stream.writeString(s); }    ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to
        virtual const char*    classID() const { return id(); }    ///< Returns the object's class ID char string. @return the class ID string
    
    private:

        Vs32    mValue;    ///< The attribute value.
    };

/**
VBentoU32 is a VBentoAttribute that holds a Vu32 value.
*/
class VBentoU32 : public VBentoAttribute
    {
    public:
    
        static const char* id() { return "vu32"; }    ///< The data type name / class ID string.
    
        VBentoU32() {}    ///< Constructs with unitinitialized name and value.
        VBentoU32(VBinaryIOStream& stream) : VBentoAttribute(stream, id()), mValue(stream.readU32()) {}    ///< Constructs by reading from stream. @param stream the stream to read
        VBentoU32(const VString& name, Vu32 i) : VBentoAttribute(name, id()), mValue(i) {}    ///< Constructs from supplied name and value.
        virtual ~VBentoU32() {}    ///< Destructor.
        
        inline Vu32    value() const { return mValue; }    ///< Returns the attribute's value. @return the value
        inline void    setValue(Vu32 i) { mValue = i; }    ///< Sets the attribute's value. @param i the attribute value
        
    protected:
        
        virtual Vs64    dataLength() const { return 4; }    ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void    writeDataToStream(VBinaryIOStream& stream) const { stream.writeU32(mValue); }    ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void    writeDataToStream(VTextIOStream& stream) const { VString s("%u", mValue); stream.writeString(s); }    ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to
        virtual const char*    classID() const { return id(); }    ///< Returns the object's class ID char string. @return the class ID string
    
    private:

        Vu32    mValue;    ///< The attribute value.
    };

/**
VBentoS64 is a VBentoAttribute that holds a Vs64 value.
*/
class VBentoS64 : public VBentoAttribute
    {
    public:
    
        static const char* id() { return "vs64"; }    ///< The data type name / class ID string.
    
        VBentoS64() {}    ///< Constructs with unitinitialized name and value.
        VBentoS64(VBinaryIOStream& stream) : VBentoAttribute(stream, id()), mValue(stream.readS64()) {}    ///< Constructs by reading from stream. @param stream the stream to read
        VBentoS64(const VString& name, Vs64 i) : VBentoAttribute(name, id()), mValue(i) {}    ///< Constructs from supplied name and value.
        virtual ~VBentoS64() {}    ///< Destructor.
        
        inline Vs64    value() const { return mValue; }    ///< Returns the attribute's value. @return the value
        inline void    setValue(Vs64 i) { mValue = i; }    ///< Sets the attribute's value. @param i the attribute value
        
    protected:
        
        virtual Vs64    dataLength() const { return 8; }    ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void    writeDataToStream(VBinaryIOStream& stream) const { stream.writeS64(mValue); }    ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void    writeDataToStream(VTextIOStream& stream) const { VString s("%lld", mValue); stream.writeString(s); }    ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to
        virtual const char*    classID() const { return id(); }    ///< Returns the object's class ID char string. @return the class ID string
    
    private:

        Vs64    mValue;    ///< The attribute value.
    };

/**
VBentoU64 is a VBentoAttribute that holds a Vu64 value.
*/
class VBentoU64 : public VBentoAttribute
    {
    public:
    
        static const char* id() { return "vu64"; }    ///< The data type name / class ID string.
    
        VBentoU64() {}    ///< Constructs with unitinitialized name and value.
        VBentoU64(VBinaryIOStream& stream) : VBentoAttribute(stream, id()), mValue(stream.readU64()) {}    ///< Constructs by reading from stream. @param stream the stream to read
        VBentoU64(const VString& name, Vu64 i) : VBentoAttribute(name, id()), mValue(i) {}    ///< Constructs from supplied name and value.
        virtual ~VBentoU64() {}    ///< Destructor.
        
        inline Vu64    value() const { return mValue; }    ///< Returns the attribute's value. @return the value
        inline void    setValue(Vu64 i) { mValue = i; }    ///< Sets the attribute's value. @param i the attribute value
        
    protected:
        
        virtual Vs64    dataLength() const { return 8; }    ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void    writeDataToStream(VBinaryIOStream& stream) const { stream.writeU64(mValue); }    ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void    writeDataToStream(VTextIOStream& stream) const { VString s("%llu", mValue); stream.writeString(s); }    ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to
        virtual const char*    classID() const { return id(); }    ///< Returns the object's class ID char string. @return the class ID string
    
    private:

        Vu64    mValue;    ///< The attribute value.
    };

/**
VBentoBool is a VBentoAttribute that holds a bool value.
*/
class VBentoBool : public VBentoAttribute
    {
    public:
    
        static const char* id() { return "bool"; }    ///< The data type name / class ID string.
    
        VBentoBool() {}    ///< Constructs with unitinitialized name and value.
        VBentoBool(VBinaryIOStream& stream) : VBentoAttribute(stream, id()), mValue(stream.readBool()) {}    ///< Constructs by reading from stream. @param stream the stream to read
        VBentoBool(const VString& name, bool b) : VBentoAttribute(name, id()), mValue(b) {}    ///< Constructs from supplied name and value.
        virtual ~VBentoBool() {}    ///< Destructor.
        
        inline bool    value() const { return mValue; }    ///< Returns the attribute's value. @return the value
        inline void    setValue(bool b) { mValue = b; }    ///< Sets the attribute's value. @param b the attribute value
        
    protected:
        
        virtual Vs64    dataLength() const { return 1; }    ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void    writeDataToStream(VBinaryIOStream& stream) const { stream.writeBool(mValue); }    ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void    writeDataToStream(VTextIOStream& stream) const { stream.writeString(mValue?"true":"false"); }    ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to
        virtual const char*    classID() const { return id(); }    ///< Returns the object's class ID char string. @return the class ID string
    
    private:

        bool    mValue;    ///< The attribute value.
    };

/**
VBentoString is a VBentoAttribute that holds a VString value.
*/
class VBentoString : public VBentoAttribute
    {
    public:
    
        static const char* id() { return "vstr"; }    ///< The data type name / class ID string.
    
        VBentoString() {}    ///< Constructs with unitinitialized name and empty string.
        VBentoString(VBinaryIOStream& stream) : VBentoAttribute(stream, id()) { stream.readString(mValue); }    ///< Constructs by reading from stream. @param stream the stream to read
        VBentoString(const VString& name, const VString& s) : VBentoAttribute(name, id()), mValue(s) {}    ///< Constructs from supplied name and value.
        virtual ~VBentoString() {}    ///< Destructor.
        
        inline const VString&    value() const { return mValue; }    ///< Returns the attribute's value. @return a reference to the value string
        inline void    setValue(const VString& s) { mValue = s; }    ///< Sets the attribute's value. @param s the attribute value
        
    protected:
        
        virtual Vs64    dataLength() const { return 4 + mValue.length(); }    ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void    writeDataToStream(VBinaryIOStream& stream) const { stream.writeString(mValue); }    ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void    writeDataToStream(VTextIOStream& stream) const { stream.writeString(mValue); }    ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to
        virtual const char*    classID() const { return id(); }    ///< Returns the object's class ID char string. @return the class ID string
    
    private:

        VString    mValue;    ///< The attribute value.
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
    
        static const char* id() { return "unkn"; }    ///< The data type name / class ID string.
    
        VBentoUnknownValue() {}    ///< Constructs with unitinitialized name and empty stream.
        VBentoUnknownValue(VBinaryIOStream& stream, Vs64 inDataLength, const VString& inDataType);    ///< Constructs by reading from stream. @param stream the stream to read @param inDataLength the length of stream data to read @param dataType the original data type value
        virtual ~VBentoUnknownValue() {}    ///< Destructor.
        
        inline const VMemoryStream&    value() const { return mValue; }    ///< Returns the attribute's value. @return a reference to the unknown-typed data stream
        
    protected:
        
        virtual Vs64    dataLength() const { return mValue.eofOffset(); }    ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void    writeDataToStream(VBinaryIOStream& stream) const;    ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void    writeDataToStream(VTextIOStream& stream) const { stream.writeString("(binary data)"); }    ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to
        virtual const char*    classID() const { return id(); }    ///< Returns the object's class ID char string. @return the class ID string
    
    private:

        VMemoryStream    mValue;    ///< The attribute value.
    };

class VBentoNode;
typedef std::vector<VBentoNode*> VBentoNodePtrVector;

/**
VBentoNode represents an object in the data hierarchy; objects can have
named/typed attributes attached to them, as well as contained (child)
objects.
*/
class VBentoNode
    {
    public:

        /**
        Reads a dynamically-sized length indicator from the stream.
        TO DO: Dynamic lengths are now supported by VBinaryIOStream itself,
        so we should just use its APIs.
        @param    stream    the stream to read from
        @return the length value as a Vs64, whatever its original size in the stream
        */
        static Vs64 readLengthFromStream(VBinaryIOStream& stream);
        /**
        Writes a dynamically-sized length indicator to the stream.
        TO DO: Dynamic lengths are now supported by VBinaryIOStream itself,
        so we should just use its APIs.
        @param    stream    the stream to write to
        @param    length    the actual length, regardless of how it will be stored in the stream
        */
        static void writeLengthToStream(VBinaryIOStream& stream, Vs64 length);
        /**
        Returns the dynamic length size of the specified length value.
        @param    length    the length value as a Vs64
        @return the number of bytes that would be used to store the length in a stream
                using the dynamic length sizing
        */
        static Vs64 getLengthOfLength(Vs64 length);
        /**
        Reads a 4-character text value from the stream, returning it as a string.
        @param    stream    the stream to read from
        @param    code    the string to place the code into
        */
        static void readFourCharCodeFromStream(VBinaryIOStream& stream, VString& code);
        /**
        Writes a 4-character text value to the stream.
        @param    stream    the stream to write to
        @param    code    the code to write, as a string; the value written is
                        truncated to 4 characters, and padded with trailing
                        spaces if less than that
        */
        static void writeFourCharCodeToStream(VBinaryIOStream& stream, const VString& code);
    
        /**
        Constructs an uninitialized object.
        */
        VBentoNode();
        /**
        Constructs a named object.
        @param    inName    the name to assign to the object
        */
        VBentoNode(const VString& inName);
        /**
        Constructs an object by read it (including its attributes and
        contained child objects) from a stream.
        @param    stream    the stream to read from
        */
        VBentoNode(VBinaryIOStream& stream);
        /**
        Destructor, deletes the object and all of its attribute objects and
        contained child objects.
        */
        virtual ~VBentoNode();
        
        /**
        Returns the object's name.
        @return    a reference to the name string
        */
        const VString&    name() const;
        
        /**
        Returns the total data length of the object, including its attributes
        and contained child objects, as it would exist as written to a binary
        data stream (via the writeToStream() method).
        @return    the total streamed data length
        */
        Vs64 calculateBinarySize() const;
        /**
        Writes the object, including its attributes and contained child
        objects, to a binary data stream.
        @param    stream    the stream to write to
        */
        void writeToStream(VBinaryIOStream& stream) const;
        /**
        Writes the object, including its attributes and contained child
        objects, to an XML text stream.
        @param    stream    the stream to write to
        @param    indentLevel    the number of spaces to indent this object's
                            level in the object hierarchy
        */
        void writeToStream(VTextIOStream& stream, int indentLevel=0) const;
        /**
        Prints the node's XML text rendering to stdout for debugging purposes.
        */
        void printXML() const;
        /**
        Prints the node's binary stream layout to stdout for debugging purposes.
        */
        void printStreamLayout();
        /**
        Prints the node's binary stream layout to stdout for debugging purposes.
        */
        void printStreamLayout(VHex& hexDump);
        
        /**
        Adds an attribute to the object. This object will delete the attribute
        object when this object is destructed.
        @param    attribute    the attribute to add
        */
        void addAttribute(VBentoAttribute* attribute);
        /**
        Adds a child to the object. This object will delete the child
        object when this object is destructed.
        @param    node    the child object node to add
        */
        void addChildNode(VBentoNode* node);
        
        /**
        Returns the vector of attribute objects attached to this object.
        @return    the attributes vector, which is a vector of pointers
        */
        const VBentoAttributePtrVector&    attributes() const;
        /**
        Returns an attribute object, searched by name and data type, that is
        attached to this object. This method does NOT search the object's
        contained child objects.
        @param    inName        the attribute name to match
        @param    inDataType    the data type name to match; typically you should
                            supply the static classID() method of the desired
                            VBentoAttribute class, for example VBentoS8::classID()
        @return    a pointer to the found attribute object, or NULL if not found
        */
        const VBentoAttribute*            findAttribute(const VString& inName, const VString& inDataType) const;

        /**
        Returns the vector of contained child objects attached to this object.
        @return    the objects vector, which is a vector of pointers
        */
        const VBentoNodePtrVector&    nodes() const;
        /**
        Returns a contained child object, searched by name, that is attached
        to this object. This method does NOT search recursively.
        @param    nodeName    the object name to match
        @return    a pointer to the found child object, or NULL if not found
        */
        const VBentoNode*        findNode(const VString& nodeName) const;
        /**
        Returns a contained child object, searched by name+attribute+dataType,
        that is attached to this object. This method does NOT search recursively.
        @param    nodeName        the object name to match
        @param    attributeName    the attribute name to match; object must have
                                an attribute with this name whose data type also
                                matches
        @param    dataType        the attribute data type to match; object must have
                                an attribute with this data type whose name also
                                matches
        @return    a pointer to the found child object, or NULL if not found
        */
        const VBentoNode*        findNode(const VString& nodeName, const VString& attributeName, const VString& inDataType) const;

        Vs8            getS8Value(const VString& inName, Vs8 defaultValue) const;    ///< Returns the value of the specified Vs8 attribute, or the supplied default value if no such Vs8 attribute exists. @param inName the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        Vs8            getS8Value(const VString& inName) const;    ///< Returns the value of the specified Vs8 attribute, or throws an exception if no such Vs8 attribute exists. @param inName the attribute name @return the found attribute's value
        Vu8            getU8Value(const VString& inName, Vu8 defaultValue) const;    ///< Returns the value of the specified Vu8 attribute, or the supplied default value if no such Vu8 attribute exists. @param inName the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        Vu8            getU8Value(const VString& inName) const;    ///< Returns the value of the specified Vu8 attribute, or throws an exception if no such Vu8 attribute exists. @param inName the attribute name @return the found attribute's value
        Vs16        getS16Value(const VString& inName, Vs16 defaultValue) const;    ///< Returns the value of the specified Vs16 attribute, or the supplied default value if no such Vs16 attribute exists. @param inName the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        Vs16        getS16Value(const VString& inName) const;    ///< Returns the value of the specified Vs16 attribute, or throws an exception if no such Vs16 attribute exists. @param inName the attribute name @return the found attribute's value
        Vu16        getU16Value(const VString& inName, Vu16 defaultValue) const;    ///< Returns the value of the specified Vu16 attribute, or the supplied default value if no such Vu16 attribute exists. @param inName the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        Vu16        getU16Value(const VString& inName) const;    ///< Returns the value of the specified Vu16 attribute, or throws an exception if no such Vu16 attribute exists. @param inName the attribute name @return the found attribute's value
        Vs32        getS32Value(const VString& inName, Vs32 defaultValue) const;    ///< Returns the value of the specified Vs32 attribute, or the supplied default value if no such Vs32 attribute exists. @param inName the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        Vs32        getS32Value(const VString& inName) const;    ///< Returns the value of the specified Vs32 attribute, or throws an exception if no such Vs32 attribute exists. @param inName the attribute name @return the found attribute's value
        Vu32        getU32Value(const VString& inName, Vu32 defaultValue) const;    ///< Returns the value of the specified Vu32 attribute, or the supplied default value if no such Vu32 attribute exists. @param inName the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        Vu32        getU32Value(const VString& inName) const;    ///< Returns the value of the specified Vu32 attribute, or throws an exception if no such Vu32 attribute exists. @param inName the attribute name @return the found attribute's value
        Vs64        getS64Value(const VString& inName, Vs64 defaultValue) const;    ///< Returns the value of the specified Vs64 attribute, or the supplied default value if no such Vs64 attribute exists. @param inName the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        Vs64        getS64Value(const VString& inName) const;    ///< Returns the value of the specified Vs64 attribute, or throws an exception if no such Vs64 attribute exists. @param inName the attribute name @return the found attribute's value
        Vu64        getU64Value(const VString& inName, Vu64 defaultValue) const;    ///< Returns the value of the specified Vu64 attribute, or the supplied default value if no such Vu64 attribute exists. @param inName the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        Vu64        getU64Value(const VString& inName) const;    ///< Returns the value of the specified Vu64 attribute, or throws an exception if no such Vu64 attribute exists. @param inName the attribute name @return the found attribute's value
        bool        getBoolValue(const VString& inName, bool defaultValue) const;    ///< Returns the value of the specified bool attribute, or the supplied default value if no such bool attribute exists. @param inName the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        bool        getBoolValue(const VString& inName) const;    ///< Returns the value of the specified bool attribute, or throws an exception if no such bool attribute exists. @param inName the attribute name @return the found attribute's value
        VString*    getStringValue(const VString& inName, VString* defaultValue) const;    ///< Returns the value of the specified string attribute, or the supplied default value if no such string attribute exists. @param inName the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        VString*    getStringValue(const VString& inName) const;    ///< Returns the value of the specified string attribute, or throws an exception if no such string attribute exists. @param inName the attribute name @return the found attribute's value
        
        void    addS8Value(const VString& inName, Vs8 value) { this->addAttribute(new VBentoS8(inName, value)); }        ///< Adds the specified attribute to the node. @param inName the attribute name @param value the attribute value
        void    addU8Value(const VString& inName, Vu8 value) { this->addAttribute(new VBentoU8(inName, value)); }        ///< Adds the specified attribute to the node. @param inName the attribute name @param value the attribute value
        void    addS16Value(const VString& inName, Vs16 value) { this->addAttribute(new VBentoS16(inName, value)); }    ///< Adds the specified attribute to the node. @param inName the attribute name @param value the attribute value
        void    addU16Value(const VString& inName, Vu16 value) { this->addAttribute(new VBentoU16(inName, value)); }    ///< Adds the specified attribute to the node. @param inName the attribute name @param value the attribute value
        void    addS32Value(const VString& inName, Vs32 value) { this->addAttribute(new VBentoS32(inName, value)); }    ///< Adds the specified attribute to the node. @param inName the attribute name @param value the attribute value
        void    addU32Value(const VString& inName, Vu32 value) { this->addAttribute(new VBentoU32(inName, value)); }    ///< Adds the specified attribute to the node. @param inName the attribute name @param value the attribute value
        void    addS64Value(const VString& inName, Vs64 value) { this->addAttribute(new VBentoS64(inName, value)); }    ///< Adds the specified attribute to the node. @param inName the attribute name @param value the attribute value
        void    addU64Value(const VString& inName, Vu64 value) { this->addAttribute(new VBentoU64(inName, value)); }    ///< Adds the specified attribute to the node. @param inName the attribute name @param value the attribute value
        void    addBoolValue(const VString& inName, bool value) { this->addAttribute(new VBentoBool(inName, value)); }    ///< Adds the specified attribute to the node. @param inName the attribute name @param value the attribute value
        void    addStringValue(const VString& inName, const VString& value) { this->addAttribute(new VBentoString(inName, value)); }    ///< Adds the specified attribute to the node. @param inName the attribute name @param value the attribute value

    private:
    
        VString                        mName;            ///< The object's name.
        VBentoAttributePtrVector    mAttributes;    ///< The object's attributes.
        VBentoNodePtrVector            mChildNodes;    ///< The object's contained child objects.

        /** Don't allow copy contructor -- default constructor has own heap memory. */
        VBentoNode(const VBentoNode&);
        /** Don't allow copy assignment -- default constructor has own heap memory. */
        void operator=(const VBentoNode&);
    };

/**
VBentoCallbackParser is provided for you to subclass if you need to read a bento object
(or complete hierarchy) from the stream "manually". You can think of this as analogous
to an XML "SAX"-like parser, where you get called as the data is read, rather than having
the whole document tree read into memory first. Typical example is for large objects that
would be inefficient to simply copy into memory in one chunk as a bento hierarchy node.
Normally you would just pick and choose which of the "xxxxComplete()" methods to override,
and/or override readAttributeData() to read the actual data. If you don't override
readAttributeData, the data is skipped over in the input stream.
*/
class VBentoCallbackParser
    {
    public:
    
        /**
        Constructs a callback parser.
        @param    stream    the input stream to read from
        */
        VBentoCallbackParser(VBinaryIOStream& stream);
        virtual ~VBentoCallbackParser() {}    ///< Destructor.

    protected:
    
        /**
        Processes a single node, including all its children (used recursively).
        @param    depth    the depth of this node in the hierarchy
        @param    stream    the input stream to read from
        */
        void processNode(int depth, VBinaryIOStream& stream);
        /**
        Processes a single attribute.
        @param    depth    the depth of this attribute's owner node in the hierarchy
        @param    stream    the input stream to read from
        */
        void processAttribute(int depth, VBinaryIOStream& stream);

        /**
        Virtual function you can override to take action when a node's header has been
        completely read; called when the node's header has been read, prior to the node's
        attributes or children being read.
        @param    depth            the node's depth in the hierarchy
        @param    length            the node's data length
        @param    numAttributes    the number attributes owned by the node
        @param    numChildren        the number of children the node has
        @param    name            the node's name
        */
        virtual void nodeHeaderComplete(int depth, Vs64 length, Vs32 numAttributes, Vs32 numChildren, const VString& name);
        /**
        Virtual function you can override to take action when a node's header and
        attributes have been completely read; called when the node's header and
        attributes have been read, prior to the node's children being read.
        @param    depth            the node's depth in the hierarchy
        @param    length            the node's data length
        @param    numAttributes    the number attributes owned by the node
        @param    numChildren        the number of children the node has
        @param    name            the node's name
        */
        virtual void nodeAttributesComplete(int depth, Vs64 length, Vs32 numAttributes, Vs32 numChildren, const VString& name);
        /**
        Virtual function you can override to take action when a node has been
        completely read; called when the node's header and attributes and
        children have all been read.
        @param    depth            the node's depth in the hierarchy
        @param    length            the node's data length
        @param    numAttributes    the number attributes owned by the node
        @param    numChildren        the number of children the node has
        @param    name            the node's name
        */
        virtual void nodeComplete(int depth, Vs64 length, Vs32 numAttributes, Vs32 numChildren, const VString& name);
        /**
        Virtual function you can override to take action when an attribute's
        header has been completely read; called when the attribute's header
        has been read, prior to its data being read.
        @param    depth            the attribute's owner node's depth in the hierarchy
        @param    length            the attribute's data length
        @param    type            the attribute's type
        @param    name            the node's name
        */
        virtual void attributeHeaderComplete(int depth, Vs64 length, const VString& type, const VString& name);
        /**
        Virtual function you can override to take action when an attribute has been
        completely read; called when the attributes's header and data have all been read.
        @param    depth            the attribute's owner node's depth in the hierarchy
        @param    length            the attribute's data length
        @param    type            the attribute's type
        @param    name            the node's name
        */
        virtual void attributeComplete(int depth, Vs64 length, const VString& type, const VString& name);
        /**
        Virtual function you can override to read the attribute's data; if you don't
        override this, the attribute data is simply skipped over.
        @param    depth            the attribute's owner node's depth in the hierarchy
        @param    stream            the input stream to read from
        @param    inDataLength    the length of the attribute data
        */
        virtual void readAttributeData(int depth, VBinaryIOStream& stream, Vu64 inDataLength);
    };

#endif /* vbento_h */
