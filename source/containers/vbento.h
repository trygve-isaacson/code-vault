/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.7
http://www.bombaydigital.com/
*/

#ifndef vbento_h
#define vbento_h

/** @file */

#include "vbinaryiostream.h"
#include "vtextiostream.h"
#include "vmemorystream.h"
#include "vhex.h"
#include "vinstant.h"

/*
VBento
Bento presents an extensible, typed, named data hierarchy; it can be used as
an abstraction of an XML DOM or a binary version of the same. A large part of
its purpose is to allow you to write data (a wire protocol or file format,
for example) such that the reader can read the data, without any effort, with
following two important backward/forward-compatibility aspects:

- unexpected data is harmlessly ignored
- missing expected data can be easily set to default values

This allows you to revise the protocol with new parameters, without breaking
the binary compatibility of old clients and old servers.

Bento Binary Stream Format

In the following description, "dyn-len" indicates a "dynamic length" value. A
dynamic length value holds a length, but the size of value depends on the
value. See VBinaryIOStream::readDynamicLength() and writeDynamicLength(). In
short, values <= 252 are encoded in a single byte; larger values are encoded
in more bytes, with a leading 0xFF, 0xFE, or 0xFD indicating whether the next
2, 4, or 8 bytes hold the value. In the stream, the length is stored in
network order ("big-endian") if it is more than 1 byte long. The purpose of
using a dynamic length is to allow for huge elements whose length requires
2 or 4 or 8 bytes to express, while keeping the cost to just 1 byte for the
vast majority of cases.

In addition, "string" indicates a string that is stored in this format:

    - dyn-len: length of text
    - text (byte stream)

A stream consists of "atom" elements. An atom is either a "node" or an
"attribute". The general format of atom is:

    - dyn-len: length of content
    - content (byte stream)

For a "node" atom, the content section is:

    - Vs32: number of attributes "na"
    - Vs32: number of child nodes "nc"
    - string: name
    - array[na] of attribute atoms
    - array[nc] of child node atoms

For an "attribute" atom, the content section is:

    - Vs8[4]: data type indicator ("vs_8" | "vu_8" | "vs16" | "vu16" | "vs32" | "vu32" | "vs64" | "vu64" | "bool" | "vstr" | "char" | "flot" | "doub")
    - string: name
    - data (type-dependent)

For the integer data types, the attribute data is 1 to 8 network order bytes ("big-endian").
For the boolean data type, the attribute data is one byte, either 1 or 0.
For the string data type, the attribute data is a string as described above.

Bento Text Format

Bento also defines a text format that it can write and read. The purpose of
this format is not to be the primary transmission format for Bento data, but
rather to provide a capability for recording and replaying messages. The
data in the Bento Text Format retains all of the attributes of the original
data: hierarchical, named, strongly typed data. The text format is optimized
for readability by inferring the the most commonly used data types from the
form of the data; this means that the data type need only be specified for
the less-used types.

Here is the Bento Text Format syntax:

node : { name attributes children }
  A node is identified by curly braces.
  A node has a name, optional attributes, and optional children.
  The children are nodes, which gives the data a hierarchical nature.
  The attributes are typically where the real "data" is located.

name : "double-quoted string"
  Node and attribute names must double-quoted strings.
  Backslash is used to escape a double-quote inside the string.
  Backslash is used to escape a backslash inside the string.

attributes : zero or more attribute
  See attribute definition below.

children : zero or more node
  Each child is simply another set of curly braces inside its parent.

attribute : [ name type = value ]
  An attribute is identified by square brackets.
  The attribute name comes first. See "name" definition above.
  The type is optional for string, bool, and int.

type : (type_abbrev)
  A type is identified by parentheses.
  The value must be one of the 4-character "data type indicator" codes described above.
  For string, bool, and int (vs32), the type can be omitted if the value is formatted
    as described below, since the type can then be inferred.

value : the text representation of the attribute's value
  For types that can be inferred:
    'vstr' values MUST be double-quoted, with escaping like names described above.
      If the type is omitted and the value is not double-quoted, an attempt will
      be made to interpret it as a 'bool' or a 'vs32', not as a string.
    'bool' values MUST NOT be double-quoted IF the type is omitted.
      If the type is omitted and the value is double-quoted, it will be interpreted
      as a string, not a 'bool'.
      The permitted values are "true" and "false".
    'vs32' values MUST NOT be double-quoted IF the type is omitted.
      If the type is omitted and the value is double-quoted, it will be interpreted
      as a string, not a 'vs32'.
  For all other types (they cannot be inferred) :
    The type must be specified.
    Double-quotes are optional.

'flot' and 'doub' values when written to Bento Text Format use 6 decimal places of
accuracy. This may place practical limits on using the text format for these data
types, if exact equality of values with many decimal places is needed. If this
becomes a problem, the method VBentoDouble::getValueAsBentoTextString() could be
changed to use a string format with more decimal places than the IEEE default of 6.
*/

class VBentoAttribute;
typedef std::vector<VBentoAttribute*> VBentoAttributePtrVector;

class VBentoNode;
typedef std::vector<VBentoNode*> VBentoNodePtrVector;

class DOMNode;
class DOMElement;

/**
VBentoNode represents an object in the data hierarchy; objects can have
named/typed attributes attached to them, as well as contained (child)
objects.
*/
class VBentoNode
    {
    public:

        // Lifecycle methods -------------------------------------------------

        /**
        Destructor, deletes the object and all of its attribute objects and
        contained child objects.
        */
        virtual ~VBentoNode();

        // Methods for creating and serializing a data hierarchy -------------

        /**
        Constructs an uninitialized object.
        */
        VBentoNode();
        /**
        Constructs a named object.
        @param    name    the name to assign to the object
        */
        VBentoNode(const VString& name);
         /**
        Adds a child to the object. This object will delete the child
        object when this object is destructed.
        @param    node    the child object node to add
        */
        void addChildNode(VBentoNode* node);
        /**
        Adds a new child to the object. This object will delete the child
        object when this object is destructed.
        @param    name    the name of the child node to create and add
        @return the newly created and added child
        */
        VBentoNode* addNewChildNode(const VString& name);

        // It is best to use this first set of data types, because they
        // are more naturally represented in other languages. Note that
        // "int" is just a more convenient name here for S32.
        void addInt(const VString& name, int value);                  ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addBool(const VString& name, bool value);                ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addString(const VString& name, const VString& value);    ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addStringIfNotEmpty(const VString& name, const VString& value);    ///< Adds the specified string to the node if its length is non-zero. @param name the attribute name @param value the attribute value
        void addChar(const VString& name, const VChar& value);        ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addDouble(const VString& name, VDouble value);           ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addDuration(const VString& name, const VDuration& value);///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addInstant(const VString& name, const VInstant& value);  ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value

        void addS8(const VString& name, Vs8 value);                   ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addU8(const VString& name, Vu8 value);                   ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addS16(const VString& name, Vs16 value);                 ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addU16(const VString& name, Vu16 value);                 ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addS32(const VString& name, Vs32 value);                 ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addU32(const VString& name, Vu32 value);                 ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addS64(const VString& name, Vs64 value);                 ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addU64(const VString& name, Vu64 value);                 ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addFloat(const VString& name, VFloat value);             ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
//        void addBinary(const VString& name, const Vu8* data, Vs64 length);///< Adds the specified attribute to the node. @param name the attribute name @param data the data buffer to add @param length the length of data to add
//        void addBinary(const VString& name, const VMemoryStream& stream);///< Adds the specified attribute to the node. @param name the attribute name @param stream the memory stream to write

       /**
        Writes the object, including its attributes and contained child
        objects, to a binary data stream.
        @param    stream    the stream to write to
        */
        void writeToStream(VBinaryIOStream& stream) const;
        /**
        Writes the object, including its attributes and contained child
        objects, to a text stream in Bento Text Format.
        @param    stream    the stream to write to
        */
        void writeToBentoTextStream(VTextIOStream& stream) const;
        /**
        Writes the object, including its attributes and contained child
        objects, to a text stream in Bento Text Format.
        @param    s    the string to write to
        */
        void writeToBentoTextString(VString& s) const;

        // Methods for de-serializing and reading a data hierarchy -----------

        /**
        Constructs an object by reading it (including its attributes and
        contained child objects) from a Bento binary data stream.
        @param    stream    the stream to read from
        */
        VBentoNode(VBinaryIOStream& stream);
        /**
        Constructs an object by reading it (including its attributes and
        contained child objects) from a Bento Text stream.
        @param    bentoTextStream    the stream to read from
        */
        VBentoNode(VTextIOStream& bentoTextStream);

        /**
        Reads the object (including its attributes and contained child objects)
        from a Bento binary data stream. This is an alternative to simply constructing the object
        with the stream as a constructor parameter. If you call this on a node
        that has already read some data from a stream (not the normal mode of
        use), this will update the node name and append further attributes and
        child nodes per the stream data.
        @param    stream    the stream to read from
        */
        void readFromStream(VBinaryIOStream& stream);
        /**
        Reads the object (including its attributes and contained child objects)
        from a Bento Text stream. This is an alternative to simply constructing the object
        with the stream as a constructor parameter. If you call this on a node
        that has already read some data from a stream (not the normal mode of
        use), this will update the node name and append further attributes and
        child nodes per the stream data.
        @param    bentoTextStream    the stream to read from
        */
        void readFromBentoTextStream(VTextIOStream& bentoTextStream);
        /**
        Reads the object (including its attributes and contained child objects)
        from a Bento Text string. This is an alternative to simply constructing the object
        with the string as a constructor parameter. If you call this on a node
        that has already read some data from a stream (not the normal mode of
        use), this will update the node name and append further attributes and
        child nodes per the stream data.
        @param    bentoTextString    the string to read from
        */
        void readFromBentoTextString(const VString& bentoTextString);

        /**
        Returns the vector of contained child objects attached to this object.
        @return    the objects vector, which is a vector of pointers
        */
        const VBentoNodePtrVector& getNodes() const;
        /**
        Returns a contained child object, searched by name, that is attached
        to this object. This method does NOT search recursively.
        @param    nodeName    the object name to match
        @return    a pointer to the found child object, or NULL if not found
        */
        const VBentoNode* findNode(const VString& nodeName) const;
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
        const VBentoNode* findNode(const VString& nodeName, const VString& attributeName, const VString& dataType) const;

        int getInt(const VString& name, int defaultValue) const; ///< Returns the value of the specified bool attribute, or the supplied default value if no such bool attribute exists. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        int getInt(const VString& name) const; ///< Returns the value of the specified bool attribute, or throws an exception if no such bool attribute exists. @param name the attribute name @return the found attribute's value
        bool getBool(const VString& name, bool defaultValue) const; ///< Returns the value of the specified bool attribute, or the supplied default value if no such bool attribute exists. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        bool getBool(const VString& name) const; ///< Returns the value of the specified bool attribute, or throws an exception if no such bool attribute exists. @param name the attribute name @return the found attribute's value
        const VString& getString(const VString& name, const VString& defaultValue) const; ///< Returns the value of the specified string attribute, or the supplied default value if no such string attribute exists. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        const VString& getString(const VString& name) const; ///< Returns the value of the specified string attribute, or throws an exception if no such string attribute exists. @param name the attribute name @return the found attribute's value
        const VChar& getChar(const VString& name, const VChar& defaultValue) const; ///< Returns the value of the specified bool attribute, or the supplied default value if no such bool attribute exists. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        const VChar& getChar(const VString& name) const; ///< Returns the value of the specified string attribute, or throws an exception if no such string attribute exists. @param name the attribute name @return the found attribute's value
        VDouble getDouble(const VString& name, VDouble defaultValue) const; ///< Returns the value of the specified bool attribute, or the supplied default value if no such bool attribute exists. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        VDouble getDouble(const VString& name) const; ///< Returns the value of the specified bool attribute, or throws an exception if no such bool attribute exists. @param name the attribute name @return the found attribute's value
        const VDuration& getDuration(const VString& name, const VDuration& defaultValue) const; ///< Returns the value of the specified bool attribute, or the supplied default value if no such bool attribute exists. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        const VDuration& getDuration(const VString& name) const; ///< Returns the value of the specified bool attribute, or throws an exception if no such bool attribute exists. @param name the attribute name @return the found attribute's value
        const VInstant& getInstant(const VString& name, const VInstant& defaultValue) const; ///< Returns the value of the specified bool attribute, or the supplied default value if no such bool attribute exists. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        const VInstant& getInstant(const VString& name) const; ///< Returns the value of the specified bool attribute, or throws an exception if no such bool attribute exists. @param name the attribute name @return the found attribute's value

        Vs8 getS8(const VString& name, Vs8 defaultValue) const;    ///< Returns the value of the specified Vs8 attribute, or the supplied default value if no such Vs8 attribute exists. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        Vs8 getS8(const VString& name) const;    ///< Returns the value of the specified Vs8 attribute, or throws an exception if no such Vs8 attribute exists. @param name the attribute name @return the found attribute's value
        Vu8 getU8(const VString& name, Vu8 defaultValue) const;    ///< Returns the value of the specified Vu8 attribute, or the supplied default value if no such Vu8 attribute exists. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        Vu8 getU8(const VString& name) const;    ///< Returns the value of the specified Vu8 attribute, or throws an exception if no such Vu8 attribute exists. @param name the attribute name @return the found attribute's value
        Vs16 getS16(const VString& name, Vs16 defaultValue) const;    ///< Returns the value of the specified Vs16 attribute, or the supplied default value if no such Vs16 attribute exists. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        Vs16 getS16(const VString& name) const;    ///< Returns the value of the specified Vs16 attribute, or throws an exception if no such Vs16 attribute exists. @param name the attribute name @return the found attribute's value
        Vu16 getU16(const VString& name, Vu16 defaultValue) const;    ///< Returns the value of the specified Vu16 attribute, or the supplied default value if no such Vu16 attribute exists. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        Vu16 getU16(const VString& name) const;    ///< Returns the value of the specified Vu16 attribute, or throws an exception if no such Vu16 attribute exists. @param name the attribute name @return the found attribute's value
        Vs32 getS32(const VString& name, Vs32 defaultValue) const;    ///< Returns the value of the specified Vs32 attribute, or the supplied default value if no such Vs32 attribute exists. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        Vs32 getS32(const VString& name) const;    ///< Returns the value of the specified Vs32 attribute, or throws an exception if no such Vs32 attribute exists. @param name the attribute name @return the found attribute's value
        Vu32 getU32(const VString& name, Vu32 defaultValue) const;    ///< Returns the value of the specified Vu32 attribute, or the supplied default value if no such Vu32 attribute exists. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        Vu32 getU32(const VString& name) const;    ///< Returns the value of the specified Vu32 attribute, or throws an exception if no such Vu32 attribute exists. @param name the attribute name @return the found attribute's value
        Vs64 getS64(const VString& name, Vs64 defaultValue) const;    ///< Returns the value of the specified Vs64 attribute, or the supplied default value if no such Vs64 attribute exists. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        Vs64 getS64(const VString& name) const;    ///< Returns the value of the specified Vs64 attribute, or throws an exception if no such Vs64 attribute exists. @param name the attribute name @return the found attribute's value
        Vu64 getU64(const VString& name, Vu64 defaultValue) const;    ///< Returns the value of the specified Vu64 attribute, or the supplied default value if no such Vu64 attribute exists. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        Vu64 getU64(const VString& name) const;    ///< Returns the value of the specified Vu64 attribute, or throws an exception if no such Vu64 attribute exists. @param name the attribute name @return the found attribute's value
        VFloat getFloat(const VString& name, VFloat defaultValue) const; ///< Returns the value of the specified VFloat attribute, or the supplied default value if no such VFloat attribute exists. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        VFloat getFloat(const VString& name) const; ///< Returns the value of the specified VFloat attribute, or throws an exception if no such VFloat attribute exists. @param name the attribute name @return the found attribute's value
//        void getBinary(const VString& name, VMemoryStream& value) const; ///< Returns the value of the specified binary data attribute, or the supplied default value if no such binary data attribute exists; the binary data is written to the supplied memory stream at its current i/o offset. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default

        /**
        Returns the node's name.
        @return    a reference to the name string
        */
        const VString& getName() const;
        /**
        Sets the node's name.
        @param name the name to give the node
        */
        void setName(const VString& name);

        // Debugging and other miscellaneous methods -------------------------

        /**
        Writes the object, including its attributes and contained child
        objects, to an XML text stream.
        @param    stream    the stream to write to
        @param    indentLevel    the number of spaces to indent this object's
                            level in the object hierarchy
        */
        void writeToXMLTextStream(VTextIOStream& stream, int indentLevel=0) const;
        /**
        Prints the node's XML text rendering to stdout for debugging purposes.
        */
        void printXML() const;
        /**
        Prints the node's binary stream layout to stdout for debugging purposes.
        */
        void printHexDump(VHex& hexDump) const;
        
        /**
        Deletes all attributes and children (recursive) from this node. This node
        remains with the same name, but everything beneath it is destroyed. Be certain
        that no one has retained pointers to child nodes nor references to
        attributes of this or any child node, before calling this method. This method
        may be useful if you build a Bento hierarchy, serialize it, and then wish to
        re-use the base node.
        */
        void clear();
        /**
        Removes all attribute references from this node. Presumably the caller is
        now responsible for those objects, including their deletion.
        */
        void orphanAttributes();
        /**
        Removes all child node references from this node. Presumably the caller is
        now responsible for those nodes, including their deletion.
        */
        void orphanNodes();
        /**
        Adopts the entire set of attributes and children from the specified node.
        The supplied node will be modified with calls to orphanAttributes and
        orphanNodes, because this node now owns those objects in memory.
        */
        void adoptFrom(VBentoNode* node);

    private:

        /**
        Returns the total content length of the object, including its attributes
        and contained child objects, as it would exist as written to a binary
        data stream (via the writeToStream() method), without the dynamic length
        indicator that precedes it.
        @return    the total streamed content data length
        */
        Vs64 _calculateContentSize() const;
        /**
        Returns the total length of the object, including its attributes
        and contained child objects, as it would exist as written to a binary
        data stream (via the writeToStream() method), including the dynamic length
        indicator that precedes the content.
        @return    the total streamed node length
        */
        Vs64 _calculateTotalSize() const;

        /**
        Adds an attribute to the object. This object will delete the attribute
        object when this object is destructed.
        @param    attribute    the attribute to add
        */
        void _addAttribute(VBentoAttribute* attribute);

        /**
        Returns the vector of attribute objects attached to this object.
        @return    the attributes vector, which is a vector of pointers
        */
        const VBentoAttributePtrVector&    _getAttributes() const;
        /**
        Returns an attribute object, searched by name and data type, that is
        attached to this object. This method does NOT search the object's
        contained child objects.
        @param    name        the attribute name to match
        @param    dataType    the data type name to match; typically you should
                            supply the static classID() method of the desired
                            VBentoAttribute class, for example VBentoS8::classID()
        @return    a pointer to the found attribute object, or NULL if not found
        */
        const VBentoAttribute* _findAttribute(const VString& name, const VString& dataType) const;

        /**
        Reads a dynamically-sized length indicator from the stream.
        @param    stream    the stream to read from
        @return the length value as a Vs64, whatever its original size in the stream
        */
        static Vs64 _readLengthFromStream(VBinaryIOStream& stream);
        /**
        Writes a dynamically-sized length indicator to the stream.
        @param    stream    the stream to write to
        @param    length    the actual length, regardless of how it will be stored in the stream
        */
        static void _writeLengthToStream(VBinaryIOStream& stream, Vs64 length);
        /**
        Returns the dynamic length size of the specified length value.
        @param    length    the length value as a Vs64
        @return the number of bytes that would be used to store the length in a stream
                using the dynamic length sizing
        */
        static Vs64 _getLengthOfLength(Vs64 length);
        /**
        Reads a 4-character text value from the stream, returning it as a string.
        @param    stream    the stream to read from
        @param    code    the string to place the code into
        */
        static void _readFourCharCodeFromStream(VBinaryIOStream& stream, VString& code);
        /**
        Writes a 4-character text value to the stream.
        @param    stream    the stream to write to
        @param    code    the code to write, as a string; the value written is
                        truncated to 4 characters, and padded with trailing
                        spaces if less than that
        */
        static void _writeFourCharCodeToStream(VBinaryIOStream& stream, const VString& code);
        /**
        Returns the length of a string as it will be encoded in a binary stream, including
        the dynamic length indicator and the text data. The dynamic length indicator's
        size varies.
        */
        static Vs64 _getBinaryStringLength(const VString& s);

		VString                     mName;          ///< The object's name.
        VBentoAttributePtrVector    mAttributes;    ///< The object's attributes.
        VBentoNodePtrVector         mChildNodes;    ///< The object's contained child objects.

        /** Don't allow copy contructor -- default constructor has own heap memory. */
        VBentoNode(const VBentoNode&);
        /** Don't allow copy assignment -- default constructor has own heap memory. */
        void operator=(const VBentoNode&);

        // These related classes use some of our private static utility functions.
        friend class VBentoAttribute;
        friend class VBentoCallbackParser;
        friend class VBentoString;
        friend class VBentoUnit;
        friend class VBentoTextNodeParser;
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
        @param    dataLength        the length of the attribute data
        */
        virtual void readAttributeData(int depth, VBinaryIOStream& stream, Vu64 dataLength);
    };

#endif /* vbento_h */
