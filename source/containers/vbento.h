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
#include "vchar.h"
#include "vexception.h"

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

    - Vs8[4]: data type indicator ("vs_8" | "vu_8" | "vs16" | "vu16" | "vs32" | "vu32" | "vs64" | "vu64" | "bool" | "vstr" | "char" | "flot" | "doub" | "dura" | "inst")
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
        */
        VBentoNode(const VBentoNode& original);
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
        Returns the vector of attribute objects attached to this object.
        @return    the attributes vector, which is a vector of pointers
        */
        const VBentoAttributePtrVector& getAttributes() const;

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
        Removes a particular child node reference from this node. Presumably the caller is
        now responsible for the node, including its deletion. If the node is not found,
        nothing happens.
        @param node the child node to remove from this node's child list
        */
        void orphanNode(const VBentoNode* node);
        /**
        Adopts the entire set of attributes and children from the specified node.
        The supplied node will be modified with calls to orphanAttributes and
        orphanNodes, because this node now owns those objects in memory.
        @param node the node from which to adopt attributes and children
        */
        void adoptFrom(VBentoNode* node);
        /**
        Updates this bento node with the attributes and children from the specified
        node, using the following rules:
        - If the source node name is non-empty, this node's name is updated with it.
        - For each source attribute, if this node has an attribute of the same
          name and type, it is replaced; otherwise the attribute is added.
        - For each source child node, if this node has a child of the same name
          it is updated (recursively, using these rules); otherwise a copy of the
          source child hierarchy is added.
        The source node is not modified and this node does not retain a reference to it.
        @param node the node from which to copy attributes and children
        */
        void updateFrom(const VBentoNode& node);

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
        Returns an attribute object, searched by name and data type, that is
        attached to this object. This method does NOT search the object's
        contained child objects.
        @param    name        the attribute name to match
        @param    dataType    the data type name to match; typically you should
                            supply the static DATA_TYPE_ID() method of the desired
                            VBentoAttribute class, for example VBentoS8::DATA_TYPE_ID()
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

        VBentoAttribute(); ///< Constructs with uninitialized name.
        VBentoAttribute(VBinaryIOStream& stream, const VString& dataType); ///< Constructs by reading from stream.
        VBentoAttribute(const VString& name, const VString& dataType); ///< Constructs with name and type. @param name the attribute name @param dataType the data type
        virtual ~VBentoAttribute(); ///< Destructor.
        
        virtual VBentoAttribute* clone() const = 0;
        virtual VBentoAttribute& operator=(const VBentoAttribute& rhs) { mName = rhs.mName; mDataType = rhs.mDataType; return *this; }

        const VString& getName() const; ///< Returns the attribute name. @return a reference to the attribute name string.
        const VString& getDataType() const; ///< Returns the data type name. @return a reference to the data type name string.
        virtual void getValueAsPlainText(VString& s) const = 0; ///< Returns the attribute value in its plain, unadorned text form.
        virtual void getValueAsString(VString& s) const = 0; ///< Returns a printable form of the attribute value.
        virtual void getValueAsBentoTextString(VString& s) const = 0; ///< Returns a Bento Text form of the attribute value.

        Vs64 calculateContentSize() const; ///< Returns the size, in bytes, of the attribute content if written to a binary stream. @return the attribute's binary size
        Vs64 calculateTotalSize() const; ///< Returns the size, in bytes, of the attribute content plus dynamic size indicator if written to a binary stream. @return the attribute's binary size
        void writeToStream(VBinaryIOStream& stream) const; ///< Writes the attribute to a binary stream. @param stream the stream to write to
        void writeToBentoTextStream(VTextIOStream& stream) const; ///< Writes the object, including its attributes and contained child objects, to a text stream in Bento Text Format. @param stream the stream to write to
        void writeToXMLTextStream(VTextIOStream& stream) const; ///< Writes the attribute to a text stream as XML. @param stream the stream to write to
        void printHexDump(VHex& hexDump) const; ///< Debugging method. Prints a hex dump of the stream. @param hexDump the hex dump formatter object

        static VBentoAttribute* newObjectFromStream(VBinaryIOStream& stream); ///< Creates a new attribute object by reading a binary stream. @param stream the stream to read from @return the new object
        static VBentoAttribute* newObjectFromStream(VTextIOStream& stream); ///< Creates a new attribute object by reading a text XML stream. @param stream the stream to read from @return the new object
        static VBentoAttribute* newObjectFromBentoTextValues(const VString& attributeName, const VString& attributeType, const VString& attributeValue);

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

        static const VString& DATA_TYPE_ID() { static const VString kID("vs_8"); return kID; } ///< The data type name / class ID string.

        VBentoS8() {} ///< Constructs with uninitialized name and value.
        VBentoS8(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream.readS8()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoS8(const VString& name, Vs8 i) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoS8() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoS8(this->getName(), mValue); }
        VBentoAttribute& operator=(const VBentoAttribute& rhs) { VBentoAttribute::operator=(rhs); mValue = static_cast<const VBentoS8&>(rhs).mValue; return *this; }

        virtual void getValueAsPlainText(VString& s) const { s.format("%d", mValue); }
        virtual void getValueAsString(VString& s) const { s.format("%d 0x%02X", mValue, mValue); }
        virtual void getValueAsBentoTextString(VString& s) const { s.format("%d", mValue); }

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

        static const VString& DATA_TYPE_ID() { static const VString kID("vu_8"); return kID; } ///< The data type name / class ID string.

        VBentoU8() {} ///< Constructs with uninitialized name and value.
        VBentoU8(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream.readU8()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoU8(const VString& name, Vu8 i) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoU8() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoU8(this->getName(), mValue); }
        VBentoAttribute& operator=(const VBentoAttribute& rhs) { VBentoAttribute::operator=(rhs); mValue = static_cast<const VBentoU8&>(rhs).mValue; return *this; }

        virtual void getValueAsPlainText(VString& s) const { s.format("%u", mValue); }
        virtual void getValueAsString(VString& s) const { s.format("%u 0x%02X", mValue, mValue); }
        virtual void getValueAsBentoTextString(VString& s) const { s.format("%u", mValue); }

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

        static const VString& DATA_TYPE_ID() { static const VString kID("vs16"); return kID; } ///< The data type name / class ID string.

        VBentoS16() {} ///< Constructs with uninitialized name and value.
        VBentoS16(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream.readS16()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoS16(const VString& name, Vs16 i) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoS16() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoS16(this->getName(), mValue); }
        VBentoAttribute& operator=(const VBentoAttribute& rhs) { VBentoAttribute::operator=(rhs); mValue = static_cast<const VBentoS16&>(rhs).mValue; return *this; }

        virtual void getValueAsPlainText(VString& s) const { s.format("%hd", mValue); }
        virtual void getValueAsString(VString& s) const { s.format("%hd 0x%04X", mValue, mValue); }
        virtual void getValueAsBentoTextString(VString& s) const { s.format("%hd", mValue); }

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

        static const VString& DATA_TYPE_ID() { static const VString kID("vu16"); return kID; } ///< The data type name / class ID string.

        VBentoU16() {} ///< Constructs with uninitialized name and value.
        VBentoU16(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream.readU16()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoU16(const VString& name, Vu16 i) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoU16() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoU16(this->getName(), mValue); }
        VBentoAttribute& operator=(const VBentoAttribute& rhs) { VBentoAttribute::operator=(rhs); mValue = static_cast<const VBentoU16&>(rhs).mValue; return *this; }

        virtual void getValueAsPlainText(VString& s) const { s.format("%hu", mValue); }
        virtual void getValueAsString(VString& s) const { s.format("%hu 0x%04X", mValue, mValue); }
        virtual void getValueAsBentoTextString(VString& s) const { s.format("%hu", mValue); }

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

        static const VString& DATA_TYPE_ID() { static const VString kID("vs32"); return kID; } ///< The data type name / class ID string.

        VBentoS32() {} ///< Constructs with uninitialized name and value.
        VBentoS32(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream.readS32()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoS32(const VString& name, Vs32 i) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoS32() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoS32(this->getName(), mValue); }
        VBentoAttribute& operator=(const VBentoAttribute& rhs) { VBentoAttribute::operator=(rhs); mValue = static_cast<const VBentoS32&>(rhs).mValue; return *this; }

        virtual void getValueAsPlainText(VString& s) const { s.format("%ld", mValue); }
        virtual void getValueAsString(VString& s) const { s.format("%ld 0x%08X", mValue, mValue); }
        virtual void getValueAsBentoTextString(VString& s) const { s.format("%ld", mValue); }

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

        static const VString& DATA_TYPE_ID() { static const VString kID("vu32"); return kID; } ///< The data type name / class ID string.

        VBentoU32() {} ///< Constructs with uninitialized name and value.
        VBentoU32(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream.readU32()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoU32(const VString& name, Vu32 i) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoU32() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoU32(this->getName(), mValue); }
        VBentoAttribute& operator=(const VBentoAttribute& rhs) { VBentoAttribute::operator=(rhs); mValue = static_cast<const VBentoU32&>(rhs).mValue; return *this; }

        virtual void getValueAsPlainText(VString& s) const { s.format("%lu", mValue); }
        virtual void getValueAsString(VString& s) const { s.format("%lu 0x%08X", mValue, mValue); }
        virtual void getValueAsBentoTextString(VString& s) const { s.format("%lu", mValue); }

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

        static const VString& DATA_TYPE_ID() { static const VString kID("vs64"); return kID; } ///< The data type name / class ID string.

        VBentoS64() {} ///< Constructs with uninitialized name and value.
        VBentoS64(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream.readS64()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoS64(const VString& name, Vs64 i) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoS64() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoS64(this->getName(), mValue); }
        VBentoAttribute& operator=(const VBentoAttribute& rhs) { VBentoAttribute::operator=(rhs); mValue = static_cast<const VBentoS64&>(rhs).mValue; return *this; }

        virtual void getValueAsPlainText(VString& s) const { s.format("%lld", mValue); }
        virtual void getValueAsString(VString& s) const { s.format("%lld 0x%016llX", mValue, mValue); }
        virtual void getValueAsBentoTextString(VString& s) const { s.format("%lld", mValue); }

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

        static const VString& DATA_TYPE_ID() { static const VString kID("vu64"); return kID; } ///< The data type name / class ID string.

        VBentoU64() {} ///< Constructs with uninitialized name and value.
        VBentoU64(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream.readU64()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoU64(const VString& name, Vu64 i) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoU64() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoU64(this->getName(), mValue); }
        VBentoAttribute& operator=(const VBentoAttribute& rhs) { VBentoAttribute::operator=(rhs); mValue = static_cast<const VBentoU64&>(rhs).mValue; return *this; }

        virtual void getValueAsPlainText(VString& s) const { s.format("%llu", mValue); }
        virtual void getValueAsString(VString& s) const { s.format("%llu 0x%016llX", mValue, mValue); }
        virtual void getValueAsBentoTextString(VString& s) const { s.format("%llu", mValue); }

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

        static const VString& DATA_TYPE_ID() { static const VString kID("bool"); return kID; } ///< The data type name / class ID string.

        VBentoBool() {} ///< Constructs with uninitialized name and value.
        VBentoBool(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream.readBool()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoBool(const VString& name, bool b) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(b) {} ///< Constructs from supplied name and value.
        virtual ~VBentoBool() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoBool(this->getName(), mValue); }
        VBentoAttribute& operator=(const VBentoAttribute& rhs) { VBentoAttribute::operator=(rhs); mValue = static_cast<const VBentoBool&>(rhs).mValue; return *this; }

        virtual void getValueAsPlainText(VString& s) const { s.format("%s", (mValue ? "true":"false")); }
        virtual void getValueAsString(VString& s) const { s.format("%s 0x%02X", (mValue ? "true":"false"), static_cast<Vu8>(mValue)); }
        virtual void getValueAsBentoTextString(VString& s) const { s.format("%s", (mValue ? "true":"false")); }

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

        static const VString& DATA_TYPE_ID() { static const VString kID("vstr"); return kID; } ///< The data type name / class ID string.

        VBentoString() {} ///< Constructs with uninitialized name and empty string.
        VBentoString(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()) { stream.readString(mValue); } ///< Constructs by reading from stream. @param stream the stream to read
        VBentoString(const VString& name, const VString& s) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(s) {} ///< Constructs from supplied name and value.
        virtual ~VBentoString() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoString(this->getName(), mValue); }
        VBentoAttribute& operator=(const VBentoAttribute& rhs) { VBentoAttribute::operator=(rhs); mValue = static_cast<const VBentoString&>(rhs).mValue; return *this; }

        virtual void getValueAsPlainText(VString& s) const { s = mValue; }
        virtual void getValueAsString(VString& s) const { s.format("\"%s\"", mValue.chars()); }
        virtual void getValueAsBentoTextString(VString& s) const { s = mValue; }

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
VBentoChar is a VBentoAttribute that holds a VChar value.
*/
class VBentoChar : public VBentoAttribute
    {
    public:

        static const VString& DATA_TYPE_ID() { static const VString kID("char"); return kID; } ///< The data type name / class ID string.

        VBentoChar() : mValue(' ') {} ///< Constructs with uninitialized name and a space char.
        VBentoChar(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()) { mValue.set(static_cast<char>(stream.readU8())); } ///< Constructs by reading from stream. @param stream the stream to read
        VBentoChar(const VString& name, const VChar& c) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(c) {} ///< Constructs from supplied name and value.
        virtual ~VBentoChar() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoChar(this->getName(), mValue); }
        VBentoAttribute& operator=(const VBentoAttribute& rhs) { VBentoAttribute::operator=(rhs); mValue = static_cast<const VBentoChar&>(rhs).mValue; return *this; }

        virtual void getValueAsPlainText(VString& s) const { s = mValue; }
        virtual void getValueAsString(VString& s) const { s.format("\"%c\"", mValue.charValue()); }
        virtual void getValueAsBentoTextString(VString& s) const { s = mValue; }

        inline const VChar& getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the value object
        inline void setValue(const VChar& c) { mValue = c; } ///< Sets the attribute's value. @param c the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 1; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToStream(VBinaryIOStream& stream) const { stream.writeU8(static_cast<Vu8>(mValue.charValue())); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void writeDataToStream(VTextIOStream& stream) const { stream.writeString(VString("%c",mValue.charValue())); } ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to

    private:

        VChar mValue; ///< The attribute value.
    };

/**
VBentoFloat is a VBentoAttribute that holds a VFloat value.
*/
class VBentoFloat : public VBentoAttribute
    {
    public:

        static const VString& DATA_TYPE_ID() { static const VString kID("flot"); return kID; } ///< The data type name / class ID string.

        VBentoFloat() : mValue(0.0f) {} ///< Constructs with uninitialized name and a 0 value.
        VBentoFloat(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()) { mValue = stream.readFloat(); } ///< Constructs by reading from stream. @param stream the stream to read
        VBentoFloat(const VString& name, VFloat f) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(f) {} ///< Constructs from supplied name and value.
        virtual ~VBentoFloat() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoFloat(this->getName(), mValue); }
        VBentoAttribute& operator=(const VBentoAttribute& rhs) { VBentoAttribute::operator=(rhs); mValue = static_cast<const VBentoFloat&>(rhs).mValue; return *this; }

        virtual void getValueAsPlainText(VString& s) const { s.format("%f", mValue); }
        virtual void getValueAsString(VString& s) const { s.format("\"%f\"", mValue); }
        virtual void getValueAsBentoTextString(VString& s) const { s.format("%f", mValue); }

        inline VFloat getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the value object
        inline void setValue(VFloat f) { mValue = f; } ///< Sets the attribute's value. @param f the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 4; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToStream(VBinaryIOStream& stream) const { stream.writeFloat(mValue); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void writeDataToStream(VTextIOStream& stream) const { stream.writeString(VString("%f",mValue)); } ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to

    private:

        VFloat mValue; ///< The attribute value.
    };

/**
VBentoDouble is a VBentoAttribute that holds a VDouble value.
*/
class VBentoDouble : public VBentoAttribute
    {
    public:

        static const VString& DATA_TYPE_ID() { static const VString kID("doub"); return kID; } ///< The data type name / class ID string.

        VBentoDouble() : mValue(0.0) {} ///< Constructs with uninitialized name and a 0 value.
        VBentoDouble(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()) { mValue = stream.readDouble(); } ///< Constructs by reading from stream. @param stream the stream to read
        VBentoDouble(const VString& name, VDouble d) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(d) {} ///< Constructs from supplied name and value.
        virtual ~VBentoDouble() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoDouble(this->getName(), mValue); }
        VBentoAttribute& operator=(const VBentoAttribute& rhs) { VBentoAttribute::operator=(rhs); mValue = static_cast<const VBentoDouble&>(rhs).mValue; return *this; }

        virtual void getValueAsPlainText(VString& s) const { s.format("%lf", mValue); }
        virtual void getValueAsString(VString& s) const { s.format("\"%lf\"", mValue); }
        virtual void getValueAsBentoTextString(VString& s) const { s.format("%lf", mValue); } // Not: %lf uses 6 decimal places by default; this limits output resolution.

        inline VDouble getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the value object
        inline void setValue(VDouble d) { mValue = d; } ///< Sets the attribute's value. @param d the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 8; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToStream(VBinaryIOStream& stream) const { stream.writeDouble(mValue); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void writeDataToStream(VTextIOStream& stream) const { stream.writeString(VString("%lf",mValue)); } ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to

    private:

        VDouble mValue; ///< The attribute value.
    };

/**
VBentoDuration is a VBentoAttribute that holds a VDuration value.
*/
class VBentoDuration : public VBentoAttribute
    {
    public:

        static const VString& DATA_TYPE_ID() { static const VString kID("dura"); return kID; } ///< The data type name / class ID string.

        VBentoDuration() : mValue() {} ///< Constructs with uninitialized name and a 0 value.
        VBentoDuration(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()) { mValue = VDuration::MILLISECOND() * stream.readS64(); } ///< Constructs by reading from stream. @param stream the stream to read
        VBentoDuration(const VString& name, const VDuration& d) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(d) {} ///< Constructs from supplied name and value.
        virtual ~VBentoDuration() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoDuration(this->getName(), mValue); }
        VBentoAttribute& operator=(const VBentoAttribute& rhs) { VBentoAttribute::operator=(rhs); mValue = static_cast<const VBentoDuration&>(rhs).mValue; return *this; }

        virtual void getValueAsPlainText(VString& s) const { s.format("%lld", mValue.getDurationMilliseconds()); }
        virtual void getValueAsString(VString& s) const { s.format("\"%lldms\"", mValue.getDurationMilliseconds()); }
        virtual void getValueAsBentoTextString(VString& s) const { s.format("%lldms", mValue.getDurationMilliseconds()); }

        inline const VDuration& getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the value object
        inline void setValue(const VDuration& d) { mValue = d; } ///< Sets the attribute's value. @param d the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 8; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToStream(VBinaryIOStream& stream) const { stream.writeS64(mValue.getDurationMilliseconds()); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void writeDataToStream(VTextIOStream& stream) const { stream.writeString(VString("%lldms",mValue.getDurationMilliseconds())); } ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to

    private:

        VDuration mValue; ///< The attribute value.
    };

/**
VBentoInstant is a VBentoAttribute that holds a VInstant value.
*/
class VBentoInstant : public VBentoAttribute
    {
    public:

        static const VString& DATA_TYPE_ID() { static const VString kID("inst"); return kID; } ///< The data type name / class ID string.

        VBentoInstant() : mValue() {} ///< Constructs with uninitialized name and the current time as value.
        VBentoInstant(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()) { mValue.setValue(stream.readS64()); } ///< Constructs by reading from stream. @param stream the stream to read
        VBentoInstant(const VString& name, const VInstant& i) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoInstant() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoInstant(this->getName(), mValue); }
        VBentoAttribute& operator=(const VBentoAttribute& rhs) { VBentoAttribute::operator=(rhs); mValue = static_cast<const VBentoInstant&>(rhs).mValue; return *this; }

        virtual void getValueAsPlainText(VString& s) const { mValue.getUTCString(s); }
        virtual void getValueAsString(VString& s) const { mValue.getUTCString(s); }
        virtual void getValueAsBentoTextString(VString& s) const { mValue.getUTCString(s); }

        inline const VInstant& getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the value object
        inline void setValue(const VInstant& i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 8; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToStream(VBinaryIOStream& stream) const { stream.writeS64(mValue.getValue()); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void writeDataToStream(VTextIOStream& stream) const { stream.writeString(VString("%lld",mValue.getValue())); } ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to

    private:

        VInstant mValue; ///< The attribute value.
    };

#if 0
/**
VBentoBinary is a VBentoAttribute that holds untyped data.
*/
class VBentoBinary : public VBentoAttribute
    {
    public:
/*    
    xxxx to do xxxx
    which of these methods need to seek in stream?
    is binary i/o complete w.r.t. writing length + data?
*/
        static const VString& DATA_TYPE_ID() { static const VString kID("bina"); return kID; } ///< The data type name / class ID string.

        VBentoBinary() : mValue(0) {} ///< Constructs with uninitialized name and a zero-length buffer.
        VBentoBinary(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(0) { Vs64 length = stream.readS64(); ::streamCopy(stream, mValue); } ///< Constructs by reading from stream. @param stream the stream to read
        VBentoBinary(const VString& name, const Vu8* data, Vs64 length) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(0) { (void) mValue.write(data, length); } ///< Constructs from supplied name and value.
        VBentoBinary(const VString& name, const VMemoryStream& stream) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(0) { ::streamCopy(stream, mValue); } ///< Constructs from supplied name and value.
        virtual ~VBentoBinary() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoBinary(this->getName(), mValue); }
        VBentoAttribute& operator=(const VBentoAttribute& rhs) { VBentoAttribute::operator=(rhs); mValue = static_cast<const VBentoBinary&>(rhs).mValue; return *this; }

        virtual void getValueAsPlainText(VString& s) const { s.format(xxxxxxxxxxxxxxxxxx); }
        virtual void getValueAsString(VString& s) const { s.format(xxxxxxxxxxxxxxxxxx); }
        virtual void getValueAsBentoTextString(VString& s) const { s.format(xxxxxxxxxxxxxxxxxx); }

//        inline getValue(VMemoryStream& value) const { ::streamCopy(mValue, value); } ///< Returns the attribute's value. @param value the memory stream to write to
//        inline void setValue(const VMemoryStream& stream) { ::streamCopy(stream, mValue); } ///< Sets the attribute's value. @param i the attribute value

    protected:

        virtual Vs64 getDataLength() const { return mValue.eofOffset(); } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToStream(VBinaryIOStream& stream) const { stream.writeS64(mValue.getValue()); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void writeDataToStream(VTextIOStream& stream) const { stream.writeString(VString("%lld",mValue.getValue())); } ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to

    private:

        VMemoryStream mValue; ///< The attribute value.
    };
#endif

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

        static const VString& DATA_TYPE_ID() { static const VString kID("unkn"); return kID; } ///< The data type name / class ID string.

        VBentoUnknownValue() {} ///< Constructs with uninitialized name and empty stream.
        VBentoUnknownValue(VBinaryIOStream& stream, Vs64 dataLength, const VString& dataType); ///< Constructs by reading from stream. @param stream the stream to read @param dataLength the length of stream data to read @param dataType the original data type value
        virtual ~VBentoUnknownValue() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { throw VException("VBentoUnknownValue does not support clone()."); }
        VBentoAttribute& operator=(const VBentoAttribute& /*rhs*/) { throw VException("VBentoUnknownValue does not support operator=()."); }

        virtual void getValueAsPlainText(VString& s) const { s = VString::EMPTY(); }
        virtual void getValueAsString(VString& s) const { VHex::bufferToHexString(mValue.getBuffer(), mValue.eofOffset(), s, true/* want leading "0x" */); }
        virtual void getValueAsBentoTextString(VString& s) const { VHex::bufferToHexString(mValue.getBuffer(), mValue.eofOffset(), s, true/* want leading "0x" */); }

        inline const VMemoryStream& getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the unknown-typed data stream

    protected:

        virtual Vs64 getDataLength() const { return mValue.eofOffset(); } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToStream(VBinaryIOStream& stream) const; ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to
        virtual void writeDataToStream(VTextIOStream& stream) const { stream.writeString("(binary data)"); } ///< Writes the object's raw data only to a text stream as XML. @param stream the stream to write to

    private:

        VMemoryStream mValue; ///< The attribute value.
    };

#endif /* vbento_h */
