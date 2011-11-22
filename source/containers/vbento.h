/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
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
#include "vgeometry.h"
#include "vcolor.h"

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

    - Vs8[4]: data type indicator ("vs_8" | "vu_8" | "vs16" | "vu16" | "vs32" | "vu32" | "vs64" | "vu64" | "bool" | "vstr" | "char" | "flot" | "doub" | "dura" | "inst" | "sizd" | "sizi" | "pt_d" | "pt_i" | "recd" | "reci" | "pold" | "poli" | "bina" )
    - string: name
    - data (type-dependent)

For the numeric data types, the attribute data is 1 to 8 network order bytes ("big-endian").
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

// Forward declarations for most attribute types.
class VBentoS32;
class VBentoBool;
class VBentoString;
class VBentoChar;
class VBentoDouble;
class VBentoDuration;
class VBentoInstant;
class VBentoSize;
class VBentoISize;
class VBentoPoint;
class VBentoIPoint;
class VBentoPoint3D;
class VBentoIPoint3D;
class VBentoLine;
class VBentoILine;
class VBentoRect;
class VBentoIRect;
class VBentoPolygon;
class VBentoIPolygon;
class VBentoColor;

class VBentoS8;
class VBentoU8;
class VBentoS16;
class VBentoU16;
class VBentoS32;
class VBentoU32;
class VBentoS64;
class VBentoU64;
class VBentoFloat;
class VBentoBinary;

// Forward declarations for array attribute types.
class VBentoStringArray;
typedef std::vector<Vs8> Vs8Array;
class VBentoS8Array;
typedef std::vector<Vs16> Vs16Array;
class VBentoS16Array;
typedef std::vector<Vs32> Vs32Array;
class VBentoS32Array;
typedef std::vector<Vs64> Vs64Array;
class VBentoS64Array;
typedef std::vector<bool> VBoolArray;
class VBentoBoolArray;
typedef std::vector<VDouble> VDoubleArray;
class VBentoDoubleArray;
class VBentoDurationArray;
class VBentoInstantArray;

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
        void addString(const VString& name, const VString& value, const VString& encoding=VString::EMPTY());    ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value @param encoding the text encoding of the value string (UTF-8 assumed if not specified)
        void addStringIfNotEmpty(const VString& name, const VString& value, const VString& encoding=VString::EMPTY());    ///< Adds the specified string to the node if its length is non-zero. @param name the attribute name @param value the attribute value @param encoding the text encoding of the value string (UTF-8 assumed if not specified)
        void addChar(const VString& name, const VChar& value);        ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addDouble(const VString& name, VDouble value);           ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addDuration(const VString& name, const VDuration& value);///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addInstant(const VString& name, const VInstant& value);  ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addSize(const VString& name, const VSize& value);        ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addISize(const VString& name, const VISize& value);      ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addPoint(const VString& name, const VPoint& value);      ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addIPoint(const VString& name, const VIPoint& value);    ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addPoint3D(const VString& name, const VPoint3D& value);  ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addIPoint3D(const VString& name, const VIPoint3D& value);///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addLine(const VString& name, const VLine& value);        ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addILine(const VString& name, const VILine& value);      ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addRect(const VString& name, const VRect& value);        ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addIRect(const VString& name, const VIRect& value);      ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addPolygon(const VString& name, const VPolygon& value);  ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addIPolygon(const VString& name, const VIPolygon& value);///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addColor(const VString& name, const VColor& value);      ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value

        void addS8(const VString& name, Vs8 value);                   ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addU8(const VString& name, Vu8 value);                   ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addS16(const VString& name, Vs16 value);                 ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addU16(const VString& name, Vu16 value);                 ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addS32(const VString& name, Vs32 value);                 ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addU32(const VString& name, Vu32 value);                 ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addS64(const VString& name, Vs64 value);                 ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addU64(const VString& name, Vu64 value);                 ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addFloat(const VString& name, VFloat value);             ///< Adds the specified attribute to the node. @param name the attribute name @param value the attribute value
        void addBinary(const VString& name, const Vu8* data, Vs64 length);///< Adds the specified attribute to the node by copying the supplied data. @param name the attribute name @param data the data buffer to add @param length the length of data to add
        void addBinary(const VString& name, Vu8* data, VMemoryStream::BufferAllocationType allocationType, bool adoptBuffer, Vs64 suppliedBufferSize, Vs64 suppliedEOFOffset);///< Adds the specified attribute to the node, using VMemoryStream::adoptBuffer() semantics. @param name the attribute name

        VBentoS8Array* addS8Array(const VString& name);                                             ///< Adds the specified attribute to the node. @param name the attribute name @return the newly added attribute
        VBentoS8Array* addS8Array(const VString& name, const Vs8Array& value);                      ///< Adds the specified attribute with array data to the node. @param name the attribute name @param value the attribute value @return the newly added attribute
        VBentoS16Array* addS16Array(const VString& name);                                           ///< Adds the specified attribute to the node. @param name the attribute name @return the newly added attribute
        VBentoS16Array* addS16Array(const VString& name, const Vs16Array& value);                   ///< Adds the specified attribute with array data to the node. @param name the attribute name @param value the attribute value @return the newly added attribute
        VBentoS32Array* addS32Array(const VString& name);                                           ///< Adds the specified attribute to the node. @param name the attribute name @return the newly added attribute
        VBentoS32Array* addS32Array(const VString& name, const Vs32Array& value);                   ///< Adds the specified attribute with array data to the node. @param name the attribute name @param value the attribute value @return the newly added attribute
        VBentoS64Array* addS64Array(const VString& name);                                           ///< Adds the specified attribute to the node. @param name the attribute name @return the newly added attribute
        VBentoS64Array* addS64Array(const VString& name, const Vs64Array& value);                   ///< Adds the specified attribute with array data to the node. @param name the attribute name @param value the attribute value @return the newly added attribute
        VBentoStringArray* addStringArray(const VString& name);                                     ///< Adds the specified attribute to the node. @param name the attribute name @return the newly added attribute
        VBentoStringArray* addStringArray(const VString& name, const VStringVector& value);         ///< Adds the specified attribute with array data to the node. @param name the attribute name @param value the attribute value @return the newly added attribute
        VBentoBoolArray* addBoolArray(const VString& name);                                         ///< Adds the specified attribute to the node. @param name the attribute name @return the newly added attribute
        VBentoBoolArray* addBoolArray(const VString& name, const VBoolArray& value);                ///< Adds the specified attribute with array data to the node. @param name the attribute name @param value the attribute value @return the newly added attribute
        VBentoDoubleArray* addDoubleArray(const VString& name);                                     ///< Adds the specified attribute to the node. @param name the attribute name @return the newly added attribute
        VBentoDoubleArray* addDoubleArray(const VString& name, const VDoubleArray& value);          ///< Adds the specified attribute with array data to the node. @param name the attribute name @param value the attribute value @return the newly added attribute
        VBentoDurationArray* addDurationArray(const VString& name);                                 ///< Adds the specified attribute to the node. @param name the attribute name @return the newly added attribute
        VBentoDurationArray* addDurationArray(const VString& name, const VDurationVector& value);   ///< Adds the specified attribute with array data to the node. @param name the attribute name @param value the attribute value @return the newly added attribute
        VBentoInstantArray* addInstantArray(const VString& name);                                   ///< Adds the specified attribute to the node. @param name the attribute name @return the newly added attribute
        VBentoInstantArray* addInstantArray(const VString& name, const VInstantVector& value);      ///< Adds the specified attribute with array data to the node. @param name the attribute name @param value the attribute value @return the newly added attribute

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
        @param    lineWrap  true if each bento node should start on its own indented line
        @param    indentDepth if lineWrap is true, the indent level depth of this node
        */
        void writeToBentoTextStream(VTextIOStream& stream, bool lineWrap=false, int indentDepth=0) const;
        /**
        Writes the object, including its attributes and contained child
        objects, to a text stream in Bento Text Format. Use some caution in calling this vs.
        writeToBentoTextStream, since the entire hierarchy must be collected into a single
        string here.
        @param    s    the string to write to
        @param    lineWrap  true if each bento node should start on its own indented line
        */
        void writeToBentoTextString(VString& s, bool lineWrap=false) const;

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
        Returns a pointer to the parent node. To change a node's parent, you must operate on the
        parent, telling it to addChildNode() or orphanNode(). You cannot directly modify the
        parent pointer.
        @return     a pointer to the parent node
        */
        VBentoNode* getParentNode() const;

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
        const VSize& getSize(const VString& name, const VSize& defaultValue) const;
        const VSize& getSize(const VString& name) const;
        const VISize& getISize(const VString& name, const VISize& defaultValue) const;
        const VISize& getISize(const VString& name) const;
        const VPoint& getPoint(const VString& name, const VPoint& defaultValue) const;
        const VPoint& getPoint(const VString& name) const;
        const VIPoint& getIPoint(const VString& name, const VIPoint& defaultValue) const;
        const VIPoint& getIPoint(const VString& name) const;
        const VPoint3D& getPoint3D(const VString& name, const VPoint3D& defaultValue) const;
        const VPoint3D& getPoint3D(const VString& name) const;
        const VIPoint3D& getIPoint3D(const VString& name, const VIPoint3D& defaultValue) const;
        const VIPoint3D& getIPoint3D(const VString& name) const;
        const VLine& getLine(const VString& name, const VLine& defaultValue) const;
        const VLine& getLine(const VString& name) const;
        const VILine& getILine(const VString& name, const VILine& defaultValue) const;
        const VILine& getILine(const VString& name) const;
        const VRect& getRect(const VString& name, const VRect& defaultValue) const;
        const VRect& getRect(const VString& name) const;
        const VIRect& getIRect(const VString& name, const VIRect& defaultValue) const;
        const VIRect& getIRect(const VString& name) const;
        const VPolygon& getPolygon(const VString& name, const VPolygon& defaultValue) const;
        const VPolygon& getPolygon(const VString& name) const;
        const VIPolygon& getIPolygon(const VString& name, const VIPolygon& defaultValue) const;
        const VIPolygon& getIPolygon(const VString& name) const;
        const VColor& getColor(const VString& name, const VColor& defaultValue) const;
        const VColor& getColor(const VString& name) const;

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
        bool getBinary(const VString& name, VReadOnlyMemoryStream& returnedReader) const; ///< Returns true and sets returnedReader if the specified binary data attribute exists, or returns false and does not touch returendReader if no such binary data attribute exists. @param name the attribute name @param defaultValue the default value to return @returnedReader a read-only memory stream that will be set to read on the attribute's binary data if it exists
        VReadOnlyMemoryStream getBinary(const VString& name) const; ///< Returns a reader on the specified binary data attribute, or throws an exception if no such binary data attribute exists. @param name the attribute name @return a reader on the found attribute's buffer

        const Vs8Array& getS8Array(const VString& name, const Vs8Array& defaultValue) const;    ///< Returns the value of the specified Vs8 array attribute, or the supplied default value if no such Vs8 attribute exists. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        const Vs8Array& getS8Array(const VString& name) const;    ///< Returns the value of the specified Vs8 array attribute, or throws an exception if no such Vs8 attribute exists. @param name the attribute name @return the found attribute's value
        const Vs16Array& getS16Array(const VString& name, const Vs16Array& defaultValue) const;    ///< Returns the value of the specified Vs16 array attribute, or the supplied default value if no such Vs16 attribute exists. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        const Vs16Array& getS16Array(const VString& name) const;    ///< Returns the value of the specified Vs16 array attribute, or throws an exception if no such Vs16 attribute exists. @param name the attribute name @return the found attribute's value
        const Vs32Array& getS32Array(const VString& name, const Vs32Array& defaultValue) const;    ///< Returns the value of the specified Vs32 array attribute, or the supplied default value if no such Vs32 attribute exists. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        const Vs32Array& getS32Array(const VString& name) const;    ///< Returns the value of the specified Vs32 array attribute, or throws an exception if no such Vs32 attribute exists. @param name the attribute name @return the found attribute's value
        const Vs64Array& getS64Array(const VString& name, const Vs64Array& defaultValue) const;    ///< Returns the value of the specified Vs64 array attribute, or the supplied default value if no such Vs64 attribute exists. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        const Vs64Array& getS64Array(const VString& name) const;    ///< Returns the value of the specified Vs64 array attribute, or throws an exception if no such Vs64 attribute exists. @param name the attribute name @return the found attribute's value
        const VStringVector& getStringArray(const VString& name, const VStringVector& defaultValue) const;    ///< Returns the value of the specified VString array attribute, or the supplied default value if no such VString attribute exists. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        const VStringVector& getStringArray(const VString& name) const;    ///< Returns the value of the specified VString array attribute, or throws an exception if no such VString attribute exists. @param name the attribute name @return the found attribute's value
        const VBoolArray& getBoolArray(const VString& name, const VBoolArray& defaultValue) const;    ///< Returns the value of the specified VBool array attribute, or the supplied default value if no such VBool attribute exists. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        const VBoolArray& getBoolArray(const VString& name) const;    ///< Returns the value of the specified VBool array attribute, or throws an exception if no such VBool attribute exists. @param name the attribute name @return the found attribute's value
        const VDoubleArray& getDoubleArray(const VString& name, const VDoubleArray& defaultValue) const;    ///< Returns the value of the specified VDouble array attribute, or the supplied default value if no such VDouble attribute exists. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        const VDoubleArray& getDoubleArray(const VString& name) const;    ///< Returns the value of the specified VDouble array attribute, or throws an exception if no such VDouble attribute exists. @param name the attribute name @return the found attribute's value
        const VDurationVector& getDurationArray(const VString& name, const VDurationVector& defaultValue) const;    ///< Returns the value of the specified VDuration array attribute, or the supplied default value if no such VDuration attribute exists. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        const VDurationVector& getDurationArray(const VString& name) const;    ///< Returns the value of the specified VDuration array attribute, or throws an exception if no such VDuration attribute exists. @param name the attribute name @return the found attribute's value
        const VInstantVector& getInstantArray(const VString& name, const VInstantVector& defaultValue) const;    ///< Returns the value of the specified VInstant array attribute, or the supplied default value if no such VInstant attribute exists. @param name the attribute name @param defaultValue the default value to return @return the found attribute's value, or the supplied default
        const VInstantVector& getInstantArray(const VString& name) const;    ///< Returns the value of the specified VInstant array attribute, or throws an exception if no such VInstant attribute exists. @param name the attribute name @return the found attribute's value

        // These setters update an existing attribute's value if the attribute exists, and adds a new attribute if it does not (just as if addXXX had been called);
        void setInt(const VString& name, int value);                  ///< Updates or adds the specified attribute of the node. @param name the attribute name @param value the attribute value
        void setBool(const VString& name, bool value);                ///< Updates or adds the specified attribute of the node. @param name the attribute name @param value the attribute value
        void setString(const VString& name, const VString& value, const VString& encoding=VString::EMPTY());    ///< Updates or adds the specified attribute of the node. @param name the attribute name @param value the attribute value @param encoding the text encoding of the value string (UTF-8 assumed if not specified)
        void setChar(const VString& name, const VChar& value);        ///< Updates or adds the specified attribute of the node. @param name the attribute name @param value the attribute value
        void setDouble(const VString& name, VDouble value);           ///< Updates or adds the specified attribute of the node. @param name the attribute name @param value the attribute value
        void setDuration(const VString& name, const VDuration& value);///< Updates or adds the specified attribute of the node. @param name the attribute name @param value the attribute value
        void setInstant(const VString& name, const VInstant& value);  ///< Updates or adds the specified attribute of the node. @param name the attribute name @param value the attribute value
        void setSize(const VString& name, const VSize& value);        ///< Updates or adds the specified attribute of the node. @param name the attribute name @param value the attribute value
        void setISize(const VString& name, const VISize& value);      ///< Updates or adds the specified attribute of the node. @param name the attribute name @param value the attribute value
        void setPoint(const VString& name, const VPoint& value);      ///< Updates or adds the specified attribute of the node. @param name the attribute name @param value the attribute value
        void setIPoint(const VString& name, const VIPoint& value);    ///< Updates or adds the specified attribute of the node. @param name the attribute name @param value the attribute value
        void setPoint3D(const VString& name, const VPoint3D& value);  ///< Updates or adds the specified attribute of the node. @param name the attribute name @param value the attribute value
        void setIPoint3D(const VString& name, const VIPoint3D& value);///< Updates or adds the specified attribute of the node. @param name the attribute name @param value the attribute value
        void setLine(const VString& name, const VLine& value);        ///< Updates or adds the specified attribute of the node. @param name the attribute name @param value the attribute value
        void setILine(const VString& name, const VILine& value);      ///< Updates or adds the specified attribute of the node. @param name the attribute name @param value the attribute value
        void setRect(const VString& name, const VRect& value);        ///< Updates or adds the specified attribute of the node. @param name the attribute name @param value the attribute value
        void setIRect(const VString& name, const VIRect& value);      ///< Updates or adds the specified attribute of the node. @param name the attribute name @param value the attribute value
        void setPolygon(const VString& name, const VPolygon& value);  ///< Updates or adds the specified attribute of the node. @param name the attribute name @param value the attribute value
        void setIPolygon(const VString& name, const VIPolygon& value);///< Updates or adds the specified attribute of the node. @param name the attribute name @param value the attribute value
        void setColor(const VString& name, const VColor& value);      ///< Updates or adds the specified attribute of the node. @param name the attribute name @param value the attribute value
        void setS64(const VString& name, Vs64 value);                 ///< Updates or adds the specified attribute of the node. @param name the attribute name @param value the attribute value

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
        @param    lineWrap  true if each bento node should start on its own indented line
        @param    indentDepth if lineWrap is true, the indent level depth of this node
        */
        void writeToXMLTextStream(VTextIOStream& stream, bool lineWrap=false, int indentDepth=0) const;
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
        This is the same as _findAttribute, but it returns a non-const pointer, and is
        itself non-const, for use in non-const code that needs to update an existing
        attribute. 
        @param    name        the attribute name to match
        @param    dataType    the data type name to match; typically you should
                            supply the static DATA_TYPE_ID() method of the desired
                            VBentoAttribute class, for example VBentoS8::DATA_TYPE_ID()
        @return    a pointer to the found attribute object, or NULL if not found
        */
        VBentoAttribute* _findMutableAttribute(const VString& name, const VString& dataType);

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
        VBentoNode*                 mParentNode;    ///< The object's parent.
        VBentoNodePtrVector         mChildNodes;    ///< The object's contained child objects.

        /** Don't allow copy assignment -- default constructor has own heap memory. */
        void operator=(const VBentoNode&);

        // Comparison operators allow Bento nodes to be sorted by name.
        friend inline bool operator< (const VBentoNode& lhs, const VBentoNode& rhs);    ///< Compares nodes using their name strings.
        friend inline bool operator<=(const VBentoNode& lhs, const VBentoNode& rhs);    ///< Compares nodes using their name strings.
        friend inline bool operator>=(const VBentoNode& lhs, const VBentoNode& rhs);    ///< Compares nodes using their name strings.
        friend inline bool operator> (const VBentoNode& lhs, const VBentoNode& rhs);    ///< Compares nodes using their name strings.

        // These related classes use some of our private static utility functions.
        friend class VBentoAttribute;
        friend class VBentoCallbackParser;
        friend class VBentoString;
        friend class VBentoBinary;
        friend class VBentoUnit;
        friend class VBentoTextNodeParser;
        friend class VBentoStringArray;
    };

inline bool operator< (const VBentoNode& lhs, const VBentoNode& rhs) { return lhs.getName() < rhs.getName(); } ///< Compares nodes using their name strings.
inline bool operator<=(const VBentoNode& lhs, const VBentoNode& rhs) { return !operator>(lhs, rhs); }
inline bool operator>=(const VBentoNode& lhs, const VBentoNode& rhs) { return !operator<(lhs, rhs); }
inline bool operator> (const VBentoNode& lhs, const VBentoNode& rhs) { return  operator<(rhs, lhs); }

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
        VBentoAttribute& operator=(const VBentoAttribute& rhs) { mName = rhs.mName; mDataType = rhs.mDataType; return *this; }

        const VString& getName() const; ///< Returns the attribute name. @return a reference to the attribute name string.
        const VString& getDataType() const; ///< Returns the data type name. @return a reference to the data type name string.

        virtual bool xmlAppearsAsArray() const { return false; } ///< True if XML output requires this attribute to use a separate child tag for its array elements; implies override of writeToXMLTextStream
        virtual void getValueAsXMLText(VString& s) const = 0; ///< Returns a string suitable for an XML attribute value, including escaping via _escapeXMLValue() if needed.
        virtual void getValueAsString(VString& s) const = 0; ///< Returns a printable form of the attribute value.
        virtual void getValueAsBentoTextString(VString& s) const = 0; ///< Returns a Bento Text form of the attribute value.

        Vs64 calculateContentSize() const; ///< Returns the size, in bytes, of the attribute content if written to a binary stream. @return the attribute's binary size
        Vs64 calculateTotalSize() const; ///< Returns the size, in bytes, of the attribute content plus dynamic size indicator if written to a binary stream. @return the attribute's binary size
        void writeToStream(VBinaryIOStream& stream) const; ///< Writes the attribute to a binary stream. @param stream the stream to write to
        void writeToBentoTextStream(VTextIOStream& stream) const; ///< Writes the object, including its attributes and contained child objects, to a text stream in Bento Text Format. @param stream the stream to write to

        virtual void writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const; ///< Writes the attribute to a text stream as XML. @param stream the stream to write to @param lineWrap true if each bento node should start on its own indented line @param indentDepth if lineWrap is true, the indent level depth of this node

        void printHexDump(VHex& hexDump) const; ///< Debugging method. Prints a hex dump of the stream. @param hexDump the hex dump formatter object

        static VBentoAttribute* newObjectFromStream(VBinaryIOStream& stream); ///< Creates a new attribute object by reading a binary stream. @param stream the stream to read from @return the new object
        static VBentoAttribute* newObjectFromStream(VTextIOStream& stream); ///< Creates a new attribute object by reading a text XML stream. @param stream the stream to read from @return the new object
        static VBentoAttribute* newObjectFromBentoTextValues(const VString& attributeName, const VString& attributeType, const VString& attributeValue, const VString& attributeQualifier);

    protected:

        virtual Vs64 getDataLength() const = 0; ///< Returns the length of this object's raw data only; pure virtual. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const = 0; ///< Writes the object's raw data only to a binary stream; pure virtual. @param stream the stream to write to

        static void _escapeXMLValue(VString& text); ///< Modifies the input XML value string by replacing any necessary characters with XML escape sequences. @param text the value text to be escaped

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

        VBentoS8() : mValue(0) {} ///< Constructs with uninitialized name and value.
        VBentoS8(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream.readS8()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoS8(const VString& name, Vs8 i) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoS8() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoS8(this->getName(), mValue); }
        VBentoS8& operator=(const VBentoS8& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual void getValueAsXMLText(VString& s) const { s = VSTRING_S8(mValue); }
        virtual void getValueAsString(VString& s) const { s.format(VSTRING_FORMATTER_S8 " 0x%02X", mValue, mValue); }
        virtual void getValueAsBentoTextString(VString& s) const { s = VSTRING_S8(mValue); }

        inline Vs8 getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(Vs8 i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 1; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { stream.writeS8(mValue); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

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

        VBentoU8() : mValue(0) {} ///< Constructs with uninitialized name and value.
        VBentoU8(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream.readU8()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoU8(const VString& name, Vu8 i) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoU8() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoU8(this->getName(), mValue); }
        VBentoU8& operator=(const VBentoU8& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual void getValueAsXMLText(VString& s) const { s = VSTRING_U8(mValue); }
        virtual void getValueAsString(VString& s) const { s.format(VSTRING_FORMATTER_U8 " 0x%02X", mValue, mValue); }
        virtual void getValueAsBentoTextString(VString& s) const { s = VSTRING_U8(mValue); }

        inline Vu8 getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(Vu8 i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 1; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { stream.writeU8(mValue); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

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

        VBentoS16() : mValue(0) {} ///< Constructs with uninitialized name and value.
        VBentoS16(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream.readS16()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoS16(const VString& name, Vs16 i) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoS16() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoS16(this->getName(), mValue); }
        VBentoS16& operator=(const VBentoS16& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual void getValueAsXMLText(VString& s) const { s = VSTRING_S16(mValue); }
        virtual void getValueAsString(VString& s) const { s.format(VSTRING_FORMATTER_S16 " 0x%04X", mValue, mValue); }
        virtual void getValueAsBentoTextString(VString& s) const { s = VSTRING_S16(mValue); }

        inline Vs16 getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(Vs16 i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 2; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { stream.writeS16(mValue); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

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

        VBentoU16() : mValue(0) {} ///< Constructs with uninitialized name and value.
        VBentoU16(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream.readU16()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoU16(const VString& name, Vu16 i) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoU16() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoU16(this->getName(), mValue); }
        VBentoU16& operator=(const VBentoU16& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual void getValueAsXMLText(VString& s) const { s = VSTRING_U16(mValue); }
        virtual void getValueAsString(VString& s) const { s.format(VSTRING_FORMATTER_U16 " 0x%04X", mValue, mValue); }
        virtual void getValueAsBentoTextString(VString& s) const { s = VSTRING_U16(mValue); }

        inline Vu16 getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(Vu16 i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 2; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { stream.writeU16(mValue); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

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

        VBentoS32() : mValue(0) {} ///< Constructs with uninitialized name and value.
        VBentoS32(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream.readS32()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoS32(const VString& name, Vs32 i) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoS32() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoS32(this->getName(), mValue); }
        VBentoS32& operator=(const VBentoS32& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual void getValueAsXMLText(VString& s) const { s = VSTRING_S32(mValue); }
        virtual void getValueAsString(VString& s) const { s.format(VSTRING_FORMATTER_S32 " 0x%08X", mValue, mValue); }
        virtual void getValueAsBentoTextString(VString& s) const { s = VSTRING_S32(mValue); }

        inline Vs32 getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(Vs32 i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 4; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { stream.writeS32(mValue); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

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

        VBentoU32() : mValue(0) {} ///< Constructs with uninitialized name and value.
        VBentoU32(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream.readU32()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoU32(const VString& name, Vu32 i) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoU32() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoU32(this->getName(), mValue); }
        VBentoU32& operator=(const VBentoU32& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual void getValueAsXMLText(VString& s) const { s = VSTRING_U32(mValue); }
        virtual void getValueAsString(VString& s) const { s.format(VSTRING_FORMATTER_U32 " 0x%08X", mValue, mValue); }
        virtual void getValueAsBentoTextString(VString& s) const { s = VSTRING_U32(mValue); }

        inline Vu32 getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(Vu32 i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 4; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { stream.writeU32(mValue); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

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

        VBentoS64() : mValue(0) {} ///< Constructs with uninitialized name and value.
        VBentoS64(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream.readS64()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoS64(const VString& name, Vs64 i) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoS64() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoS64(this->getName(), mValue); }
        VBentoS64& operator=(const VBentoS64& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual void getValueAsXMLText(VString& s) const { s = VSTRING_S64(mValue); }
        virtual void getValueAsString(VString& s) const { s.format(VSTRING_FORMATTER_S64 " 0x%016llX", mValue, mValue); }
        virtual void getValueAsBentoTextString(VString& s) const { s = VSTRING_S64(mValue); }

        inline Vs64 getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(Vs64 i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 8; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { stream.writeS64(mValue); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

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

        VBentoU64() : mValue(0) {} ///< Constructs with uninitialized name and value.
        VBentoU64(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream.readU64()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoU64(const VString& name, Vu64 i) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoU64() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoU64(this->getName(), mValue); }
        VBentoU64& operator=(const VBentoU64& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual void getValueAsXMLText(VString& s) const { s = VSTRING_U64(mValue); }
        virtual void getValueAsString(VString& s) const { s.format(VSTRING_FORMATTER_U64 " 0x%016llX", mValue, mValue); }
        virtual void getValueAsBentoTextString(VString& s) const { s = VSTRING_U64(mValue); }

        inline Vu64 getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(Vu64 i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 8; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { stream.writeU64(mValue); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

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

        VBentoBool() : mValue(false) {} ///< Constructs with uninitialized name and value.
        VBentoBool(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream.readBool()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoBool(const VString& name, bool b) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(b) {} ///< Constructs from supplied name and value.
        virtual ~VBentoBool() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoBool(this->getName(), mValue); }
        VBentoBool& operator=(const VBentoBool& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual void getValueAsXMLText(VString& s) const { s = VSTRING_BOOL(mValue); }
        virtual void getValueAsString(VString& s) const { s.format("%s 0x%02X", (mValue ? "true":"false"), static_cast<Vu8>(mValue)); }
        virtual void getValueAsBentoTextString(VString& s) const { s = VSTRING_BOOL(mValue); }

        inline bool getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(bool b) { mValue = b; } ///< Sets the attribute's value. @param b the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 1; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { stream.writeBool(mValue); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

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

        VBentoString() : mValue() {} ///< Constructs with uninitialized name and empty string.
        VBentoString(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mEncoding(stream.readString()), mValue(stream.readString()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoString(const VString& name, const VString& s, const VString& encoding) : VBentoAttribute(name, DATA_TYPE_ID()), mEncoding(encoding), mValue(s) {} ///< Constructs from supplied name and value.
        virtual ~VBentoString() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoString(this->getName(), mValue, mEncoding); }
        VBentoString& operator=(const VBentoString& rhs) { VBentoAttribute::operator=(rhs); mEncoding = rhs.mEncoding; mValue = rhs.mValue; return *this; }

        virtual void getValueAsXMLText(VString& s) const { s = mValue; VBentoAttribute::_escapeXMLValue(s); }
        virtual void getValueAsString(VString& s) const { s.format("\"%s\"", mValue.chars()); }
        virtual void getValueAsBentoTextString(VString& s) const { s = mValue; }

        inline const VString& getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the value string
        inline void setValue(const VString& s) { mValue = s; } ///< Sets the attribute's value. @param s the attribute value
        
        inline const VString& getEncoding() const { return mEncoding; } ///< Returns the value's encoding name; empty implies UTF-8. @return a reference to the encoding name
        inline void setEncoding(const VString& encoding) { mEncoding = encoding; } ///< Sets the the value's encoding name; empty implies UTF-8. @param encoding the attribute value's encoding

    protected:

        virtual Vs64 getDataLength() const { return VBentoNode::_getBinaryStringLength(mEncoding) + VBentoNode::_getBinaryStringLength(mValue); } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { stream.writeString(mEncoding); stream.writeString(mValue); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

    private:

        VString mEncoding; ///< The optional indicator of the value's encoding (assumed UTF-8 otherwise).
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
        VBentoChar(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(static_cast<char>(stream.readU8())) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoChar(const VString& name, const VChar& c) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(c) {} ///< Constructs from supplied name and value.
        virtual ~VBentoChar() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoChar(this->getName(), mValue); }
        VBentoChar& operator=(const VBentoChar& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual void getValueAsXMLText(VString& s) const { s = mValue; VBentoAttribute::_escapeXMLValue(s); }
        virtual void getValueAsString(VString& s) const { s.format("\"%c\"", mValue.charValue()); }
        virtual void getValueAsBentoTextString(VString& s) const { s = mValue; }

        inline const VChar& getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the value object
        inline void setValue(const VChar& c) { mValue = c; } ///< Sets the attribute's value. @param c the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 1; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { stream.writeU8(static_cast<Vu8>(mValue.charValue())); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

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
        VBentoFloat(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream.readFloat()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoFloat(const VString& name, VFloat f) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(f) {} ///< Constructs from supplied name and value.
        virtual ~VBentoFloat() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoFloat(this->getName(), mValue); }
        VBentoFloat& operator=(const VBentoFloat& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual void getValueAsXMLText(VString& s) const { s = VSTRING_FLOAT(mValue); }
        virtual void getValueAsString(VString& s) const { s = VSTRING_FLOAT(mValue); }
        virtual void getValueAsBentoTextString(VString& s) const { s = VSTRING_FLOAT(mValue); }

        inline VFloat getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the value object
        inline void setValue(VFloat f) { mValue = f; } ///< Sets the attribute's value. @param f the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 4; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { stream.writeFloat(mValue); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

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
        VBentoDouble(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream.readDouble()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoDouble(const VString& name, VDouble d) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(d) {} ///< Constructs from supplied name and value.
        virtual ~VBentoDouble() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoDouble(this->getName(), mValue); }
        VBentoDouble& operator=(const VBentoDouble& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual void getValueAsXMLText(VString& s) const { s = VSTRING_DOUBLE(mValue); }
        virtual void getValueAsString(VString& s) const { s = VSTRING_DOUBLE(mValue); }
        virtual void getValueAsBentoTextString(VString& s) const { s = VSTRING_DOUBLE(mValue); } // Note: %lf uses 6 decimal places by default; this limits output resolution.

        inline VDouble getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the value object
        inline void setValue(VDouble d) { mValue = d; } ///< Sets the attribute's value. @param d the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 8; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { stream.writeDouble(mValue); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

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
        VBentoDuration(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(VDuration::MILLISECOND() * stream.readS64()) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoDuration(const VString& name, const VDuration& d) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(d) {} ///< Constructs from supplied name and value.
        virtual ~VBentoDuration() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoDuration(this->getName(), mValue); }
        VBentoDuration& operator=(const VBentoDuration& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual void getValueAsXMLText(VString& s) const { s = (mValue.isSpecific() ? (VSTRING_S64(mValue.getDurationMilliseconds())) : (mValue.getDurationString())); }
        virtual void getValueAsString(VString& s) const { s.format(VSTRING_FORMATTER_S64 "ms", mValue.getDurationMilliseconds()); }
        virtual void getValueAsBentoTextString(VString& s) const { s.format(VSTRING_FORMATTER_S64 "ms", mValue.getDurationMilliseconds()); }

        inline const VDuration& getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the value object
        inline void setValue(const VDuration& d) { mValue = d; } ///< Sets the attribute's value. @param d the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 8; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { stream.writeS64(mValue.getDurationMilliseconds()); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

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
        VBentoInstant(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(VInstant::instantFromRawValue(stream.readS64())) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoInstant(const VString& name, const VInstant& i) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(i) {} ///< Constructs from supplied name and value.
        virtual ~VBentoInstant() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoInstant(this->getName(), mValue); }
        VBentoInstant& operator=(const VBentoInstant& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual void getValueAsXMLText(VString& s) const { mValue.getUTCString(s); }
        virtual void getValueAsString(VString& s) const { mValue.getUTCString(s); }
        virtual void getValueAsBentoTextString(VString& s) const { mValue.getUTCString(s); }

        inline const VInstant& getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the value object
        inline void setValue(const VInstant& i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 8; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { stream.writeS64(mValue.getValue()); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

    private:

        VInstant mValue; ///< The attribute value.
    };

/**
VBentoSize is a VBentoAttribute that holds a VSize value.
*/
class VBentoSize : public VBentoAttribute
    {
    public:

        static const VString& DATA_TYPE_ID() { static const VString kID("sizd"); return kID; } ///< The data type name / class ID string.

        VBentoSize() : mValue() {} ///< Constructs with uninitialized name and the current time as value.
        VBentoSize(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoSize(const VString& name, const VSize& s) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(s) {} ///< Constructs from supplied name and value.
        virtual ~VBentoSize() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoSize(this->getName(), mValue); }
        VBentoSize& operator=(const VBentoSize& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual bool xmlAppearsAsArray() const { return true; } // Complex attribute requires its own child tag, formatted via writeToXMLTextStream().
        virtual void writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const; ///< Writes the attribute to a text stream as XML. @param stream the stream to write to @param lineWrap true if each bento node should start on its own indented line @param indentDepth if lineWrap is true, the indent level depth of this node

        virtual void getValueAsXMLText(VString&) const {} // n/a, since xmlAppearsAsArray() returns true for this class
        virtual void getValueAsString(VString& s) const { s.format("%lf,%lf", mValue.getWidth(), mValue.getHeight()); }
        virtual void getValueAsBentoTextString(VString& s) const { s.format("%lf,%lf", mValue.getWidth(), mValue.getHeight()); }

        inline const VSize& getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the value object
        inline void setValue(const VSize& i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 16; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { mValue.writeToStream(stream); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

    private:

        VSize mValue; ///< The attribute value.
    };

/**
VBentoSize is a VBentoAttribute that holds a VISize value.
*/
class VBentoISize : public VBentoAttribute
    {
    public:

        static const VString& DATA_TYPE_ID() { static const VString kID("sizi"); return kID; } ///< The data type name / class ID string.

        VBentoISize() : mValue() {} ///< Constructs with uninitialized name and the current time as value.
        VBentoISize(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoISize(const VString& name, const VISize& s) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(s) {} ///< Constructs from supplied name and value.
        virtual ~VBentoISize() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoISize(this->getName(), mValue); }
        VBentoISize& operator=(const VBentoISize& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual bool xmlAppearsAsArray() const { return true; } // Complex attribute requires its own child tag, formatted via writeToXMLTextStream().
        virtual void writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const; ///< Writes the attribute to a text stream as XML. @param stream the stream to write to @param lineWrap true if each bento node should start on its own indented line @param indentDepth if lineWrap is true, the indent level depth of this node

        virtual void getValueAsXMLText(VString&) const {} // n/a, since xmlAppearsAsArray() returns true for this class
        virtual void getValueAsString(VString& s) const { s.format("%d,%d", mValue.getWidth(), mValue.getHeight()); }
        virtual void getValueAsBentoTextString(VString& s) const { s.format("%d,%d", mValue.getWidth(), mValue.getHeight()); }

        inline const VISize& getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the value object
        inline void setValue(const VISize& i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 8; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { mValue.writeToStream(stream); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

    private:

        VISize mValue; ///< The attribute value.
    };

/**
VBentoPoint is a VBentoAttribute that holds a VPoint value.
*/
class VBentoPoint : public VBentoAttribute
    {
    public:

        static const VString& DATA_TYPE_ID() { static const VString kID("pt_d"); return kID; } ///< The data type name / class ID string.

        VBentoPoint() : mValue() {} ///< Constructs with uninitialized name and the current time as value.
        VBentoPoint(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoPoint(const VString& name, const VPoint& p) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(p) {} ///< Constructs from supplied name and value.
        virtual ~VBentoPoint() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoPoint(this->getName(), mValue); }
        VBentoPoint& operator=(const VBentoPoint& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual bool xmlAppearsAsArray() const { return true; } // Complex attribute requires its own child tag, formatted via writeToXMLTextStream().
        virtual void writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const; ///< Writes the attribute to a text stream as XML. @param stream the stream to write to @param lineWrap true if each bento node should start on its own indented line @param indentDepth if lineWrap is true, the indent level depth of this node

        virtual void getValueAsXMLText(VString&) const {} // n/a, since xmlAppearsAsArray() returns true for this class
        virtual void getValueAsString(VString& s) const { s.format("%lf,%lf", mValue.getX(), mValue.getY()); }
        virtual void getValueAsBentoTextString(VString& s) const { s.format("%lf,%lf", mValue.getX(), mValue.getY()); }

        inline const VPoint& getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the value object
        inline void setValue(const VPoint& i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 16; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { mValue.writeToStream(stream); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

    private:

        VPoint mValue; ///< The attribute value.
    };

/**
VBentoIPoint is a VBentoAttribute that holds a VIPoint value.
*/
class VBentoIPoint : public VBentoAttribute
    {
    public:

        static const VString& DATA_TYPE_ID() { static const VString kID("pt_i"); return kID; } ///< The data type name / class ID string.

        VBentoIPoint() : mValue() {} ///< Constructs with uninitialized name and the current time as value.
        VBentoIPoint(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoIPoint(const VString& name, const VIPoint& s) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(s) {} ///< Constructs from supplied name and value.
        virtual ~VBentoIPoint() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoIPoint(this->getName(), mValue); }
        VBentoIPoint& operator=(const VBentoIPoint& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual bool xmlAppearsAsArray() const { return true; } // Complex attribute requires its own child tag, formatted via writeToXMLTextStream().
        virtual void writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const; ///< Writes the attribute to a text stream as XML. @param stream the stream to write to @param lineWrap true if each bento node should start on its own indented line @param indentDepth if lineWrap is true, the indent level depth of this node

        virtual void getValueAsXMLText(VString&) const {} // n/a, since xmlAppearsAsArray() returns true for this class
        virtual void getValueAsString(VString& s) const { s.format("%d,%d", mValue.getX(), mValue.getY()); }
        virtual void getValueAsBentoTextString(VString& s) const { s.format("%d,%d", mValue.getX(), mValue.getY()); }

        inline const VIPoint& getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the value object
        inline void setValue(const VIPoint& i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 8; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { mValue.writeToStream(stream); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

    private:

        VIPoint mValue; ///< The attribute value.
    };

/**
VBentoPoint3D is a VBentoAttribute that holds a VPoint3D value.
*/
class VBentoPoint3D : public VBentoAttribute
    {
    public:

        static const VString& DATA_TYPE_ID() { static const VString kID("pt3d"); return kID; } ///< The data type name / class ID string.

        VBentoPoint3D() : mValue() {} ///< Constructs with uninitialized name and the current time as value.
        VBentoPoint3D(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoPoint3D(const VString& name, const VPoint3D& p) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(p) {} ///< Constructs from supplied name and value.
        virtual ~VBentoPoint3D() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoPoint3D(this->getName(), mValue); }
        VBentoPoint3D& operator=(const VBentoPoint3D& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual bool xmlAppearsAsArray() const { return true; } // Complex attribute requires its own child tag, formatted via writeToXMLTextStream().
        virtual void writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const; ///< Writes the attribute to a text stream as XML. @param stream the stream to write to @param lineWrap true if each bento node should start on its own indented line @param indentDepth if lineWrap is true, the indent level depth of this node

        virtual void getValueAsXMLText(VString&) const {} // n/a, since xmlAppearsAsArray() returns true for this class
        virtual void getValueAsString(VString& s) const { s.format("%lf,%lf,%lf", mValue.getX(), mValue.getY(), mValue.getZ()); }
        virtual void getValueAsBentoTextString(VString& s) const { s.format("%lf,%lf,%lf", mValue.getX(), mValue.getY(), mValue.getZ()); }

        inline const VPoint3D& getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the value object
        inline void setValue(const VPoint3D& i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 24; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { mValue.writeToStream(stream); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

    private:

        VPoint3D mValue; ///< The attribute value.
    };

/**
VBentoIPoint3D is a VBentoAttribute that holds a VIPoint3D value.
*/
class VBentoIPoint3D : public VBentoAttribute
    {
    public:

        static const VString& DATA_TYPE_ID() { static const VString kID("pt3i"); return kID; } ///< The data type name / class ID string.

        VBentoIPoint3D() : mValue() {} ///< Constructs with uninitialized name and the current time as value.
        VBentoIPoint3D(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoIPoint3D(const VString& name, const VIPoint3D& s) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(s) {} ///< Constructs from supplied name and value.
        virtual ~VBentoIPoint3D() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoIPoint3D(this->getName(), mValue); }
        VBentoIPoint3D& operator=(const VBentoIPoint3D& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual bool xmlAppearsAsArray() const { return true; } // Complex attribute requires its own child tag, formatted via writeToXMLTextStream().
        virtual void writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const; ///< Writes the attribute to a text stream as XML. @param stream the stream to write to @param lineWrap true if each bento node should start on its own indented line @param indentDepth if lineWrap is true, the indent level depth of this node

        virtual void getValueAsXMLText(VString&) const {} // n/a, since xmlAppearsAsArray() returns true for this class
        virtual void getValueAsString(VString& s) const { s.format("%d,%d,%d", mValue.getX(), mValue.getY(), mValue.getZ()); }
        virtual void getValueAsBentoTextString(VString& s) const { s.format("%d,%d,%d", mValue.getX(), mValue.getY(), mValue.getZ()); }

        inline const VIPoint3D& getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the value object
        inline void setValue(const VIPoint3D& i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 12; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { mValue.writeToStream(stream); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

    private:

        VIPoint3D mValue; ///< The attribute value.
    };

/**
VBentoLine is a VBentoAttribute that holds a VLine value.
*/
class VBentoLine : public VBentoAttribute
    {
    public:

        static const VString& DATA_TYPE_ID() { static const VString kID("line"); return kID; } ///< The data type name / class ID string.

        VBentoLine() : mValue() {} ///< Constructs with uninitialized name and the current time as value.
        VBentoLine(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoLine(const VString& name, const VLine& v) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(v) {} ///< Constructs from supplied name and value.
        virtual ~VBentoLine() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoLine(this->getName(), mValue); }
        VBentoLine& operator=(const VBentoLine& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual bool xmlAppearsAsArray() const { return true; } // Complex attribute requires its own child tag, formatted via writeToXMLTextStream().
        virtual void writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const; ///< Writes the attribute to a text stream as XML. @param stream the stream to write to @param lineWrap true if each bento node should start on its own indented line @param indentDepth if lineWrap is true, the indent level depth of this node

        virtual void getValueAsXMLText(VString&) const {} // n/a, since xmlAppearsAsArray() returns true for this class
        virtual void getValueAsString(VString& s) const { s.format("%lf,%lf:%lf,%lf", mValue.getP1().getX(), mValue.getP1().getY(), mValue.getP2().getX(), mValue.getP2().getY()); }
        virtual void getValueAsBentoTextString(VString& s) const { s.format("%lf,%lf:%lf,%lf", mValue.getP1().getX(), mValue.getP1().getY(), mValue.getP2().getX(), mValue.getP2().getY()); }

        inline const VLine& getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the value object
        inline void setValue(const VLine& i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 32; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { mValue.writeToStream(stream); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

    private:

        VLine mValue; ///< The attribute value.
    };

/**
VBentoILine is a VBentoAttribute that holds a VILine value.
*/
class VBentoILine : public VBentoAttribute
    {
    public:

        static const VString& DATA_TYPE_ID() { static const VString kID("lini"); return kID; } ///< The data type name / class ID string.

        VBentoILine() : mValue() {} ///< Constructs with uninitialized name and the current time as value.
        VBentoILine(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoILine(const VString& name, const VILine& v) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(v) {} ///< Constructs from supplied name and value.
        virtual ~VBentoILine() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoILine(this->getName(), mValue); }
        VBentoILine& operator=(const VBentoILine& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual bool xmlAppearsAsArray() const { return true; } // Complex attribute requires its own child tag, formatted via writeToXMLTextStream().
        virtual void writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const; ///< Writes the attribute to a text stream as XML. @param stream the stream to write to @param lineWrap true if each bento node should start on its own indented line @param indentDepth if lineWrap is true, the indent level depth of this node

        virtual void getValueAsXMLText(VString&) const {} // n/a, since xmlAppearsAsArray() returns true for this class
        virtual void getValueAsString(VString& s) const { s.format("%d,%d:%d,%d", mValue.getP1().getX(), mValue.getP1().getY(), mValue.getP2().getX(), mValue.getP2().getY()); }
        virtual void getValueAsBentoTextString(VString& s) const { s.format("%d,%d:%d,%d", mValue.getP1().getX(), mValue.getP1().getY(), mValue.getP2().getX(), mValue.getP2().getY()); }

        inline const VILine& getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the value object
        inline void setValue(const VILine& i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 16; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { mValue.writeToStream(stream); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

    private:

        VILine mValue; ///< The attribute value.
    };

/**
VBentoRect is a VBentoAttribute that holds a VRect value.
*/
class VBentoRect : public VBentoAttribute
    {
    public:

        static const VString& DATA_TYPE_ID() { static const VString kID("recd"); return kID; } ///< The data type name / class ID string.

        VBentoRect() : mValue() {} ///< Constructs with uninitialized name and the current time as value.
        VBentoRect(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoRect(const VString& name, const VRect& p) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(p) {} ///< Constructs from supplied name and value.
        virtual ~VBentoRect() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoRect(this->getName(), mValue); }
        VBentoRect& operator=(const VBentoRect& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual bool xmlAppearsAsArray() const { return true; } // Complex attribute requires its own child tag, formatted via writeToXMLTextStream().
        virtual void writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const; ///< Writes the attribute to a text stream as XML. @param stream the stream to write to @param lineWrap true if each bento node should start on its own indented line @param indentDepth if lineWrap is true, the indent level depth of this node

        virtual void getValueAsXMLText(VString&) const {} // n/a, since xmlAppearsAsArray() returns true for this class
        virtual void getValueAsString(VString& s) const { s.format("%lf,%lf:%lf*%lf", mValue.getLeft(), mValue.getTop(), mValue.getWidth(), mValue.getHeight()); }
        virtual void getValueAsBentoTextString(VString& s) const { s.format("%lf,%lf:%lf*%lf", mValue.getLeft(), mValue.getTop(), mValue.getWidth(), mValue.getHeight()); }

        inline const VRect& getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the value object
        inline void setValue(const VRect& i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 32; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { mValue.writeToStream(stream); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

    private:

        VRect mValue; ///< The attribute value.
    };

/**
VBentoIRect is a VBentoAttribute that holds a VIRect value.
*/
class VBentoIRect : public VBentoAttribute
    {
    public:

        static const VString& DATA_TYPE_ID() { static const VString kID("reci"); return kID; } ///< The data type name / class ID string.

        VBentoIRect() : mValue() {} ///< Constructs with uninitialized name and the current time as value.
        VBentoIRect(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoIRect(const VString& name, const VIRect& s) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(s) {} ///< Constructs from supplied name and value.
        virtual ~VBentoIRect() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoIRect(this->getName(), mValue); }
        VBentoIRect& operator=(const VBentoIRect& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual bool xmlAppearsAsArray() const { return true; } // Complex attribute requires its own child tag, formatted via writeToXMLTextStream().
        virtual void writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const; ///< Writes the attribute to a text stream as XML. @param stream the stream to write to @param lineWrap true if each bento node should start on its own indented line @param indentDepth if lineWrap is true, the indent level depth of this node

        virtual void getValueAsXMLText(VString&) const {} // n/a, since xmlAppearsAsArray() returns true for this class
        virtual void getValueAsString(VString& s) const { s.format("%d,%d:%d*%d", mValue.getLeft(), mValue.getTop(), mValue.getWidth(), mValue.getHeight()); }
        virtual void getValueAsBentoTextString(VString& s) const { s.format("%d,%d:%d*%d", mValue.getLeft(), mValue.getTop(), mValue.getWidth(), mValue.getHeight()); }

        inline const VIRect& getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the value object
        inline void setValue(const VIRect& i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 16; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { mValue.writeToStream(stream); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

    private:

        VIRect mValue; ///< The attribute value.
    };

/**
VBentoPolygon is a VBentoAttribute that holds a VPolygon value.
*/
class VBentoPolygon : public VBentoAttribute
    {
    public:

        static const VString& DATA_TYPE_ID() { static const VString kID("pold"); return kID; } ///< The data type name / class ID string.

        VBentoPolygon() : mValue() {} ///< Constructs with uninitialized name and the current time as value.
        VBentoPolygon(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoPolygon(const VString& name, const VPolygon& p) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(p) {} ///< Constructs from supplied name and value.
        virtual ~VBentoPolygon() {} ///< Destructor.
        
        virtual VBentoAttribute* clone() const { return new VBentoPolygon(this->getName(), mValue); }
        VBentoPolygon& operator=(const VBentoPolygon& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        static void readPolygonFromBentoTextString(const VString& s, VPolygon& p);

        virtual bool xmlAppearsAsArray() const { return true; } // Complex attribute requires its own child tag, formatted via writeToXMLTextStream().
        virtual void writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const; ///< Writes the attribute to a text stream as XML. @param stream the stream to write to @param lineWrap true if each bento node should start on its own indented line @param indentDepth if lineWrap is true, the indent level depth of this node

        virtual void getValueAsXMLText(VString&) const {} // n/a, since xmlAppearsAsArray() returns true for this class
        virtual void getValueAsString(VString& s) const { VBentoPolygon::_formatPolygonAsBentoTextString(mValue, s); }
        virtual void getValueAsBentoTextString(VString& s) const { VBentoPolygon::_formatPolygonAsBentoTextString(mValue, s); }

        inline const VPolygon& getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the value object
        inline void setValue(const VPolygon& i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 4 + (16*mValue.getNumPoints()); } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { mValue.writeToStream(stream); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

    private:
    
        static void _formatPolygonAsBentoTextString(const VPolygon& p, VString& s);

        VPolygon mValue; ///< The attribute value.
    };

/**
VBentoPolygon is a VBentoAttribute that holds a VIPolygon value.
*/
class VBentoIPolygon : public VBentoAttribute
    {
    public:

        static const VString& DATA_TYPE_ID() { static const VString kID("poli"); return kID; } ///< The data type name / class ID string.

        VBentoIPolygon() : mValue() {} ///< Constructs with uninitialized name and the current time as value.
        VBentoIPolygon(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoIPolygon(const VString& name, const VIPolygon& s) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(s) {} ///< Constructs from supplied name and value.
        virtual ~VBentoIPolygon() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoIPolygon(this->getName(), mValue); }
        VBentoIPolygon& operator=(const VBentoIPolygon& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        static void readPolygonFromBentoTextString(const VString& s, VIPolygon& p);

        virtual bool xmlAppearsAsArray() const { return true; } // Complex attribute requires its own child tag, formatted via writeToXMLTextStream().
        virtual void writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int indentDepth) const; ///< Writes the attribute to a text stream as XML. @param stream the stream to write to @param lineWrap true if each bento node should start on its own indented line @param indentDepth if lineWrap is true, the indent level depth of this node

        virtual void getValueAsXMLText(VString&) const {} // n/a, since xmlAppearsAsArray() returns true for this class
        virtual void getValueAsString(VString& s) const { VBentoIPolygon::_formatPolygonAsBentoTextString(mValue, s); }
        virtual void getValueAsBentoTextString(VString& s) const { VBentoIPolygon::_formatPolygonAsBentoTextString(mValue, s); }

        inline const VIPolygon& getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the value object
        inline void setValue(const VIPolygon& i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 4 + (8*mValue.getNumPoints()); } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { mValue.writeToStream(stream); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

    private:

        static void _formatPolygonAsBentoTextString(const VIPolygon& p, VString& s);

        VIPolygon mValue; ///< The attribute value.
    };

/**
VBentoColor is a VBentoAttribute that holds a VColor value.
*/
class VBentoColor : public VBentoAttribute
    {
    public:

        static const VString& DATA_TYPE_ID() { static const VString kID("rgba"); return kID; } ///< The data type name / class ID string.

        VBentoColor() : mValue() {} ///< Constructs with uninitialized name and the default value.
        VBentoColor(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(stream) {} ///< Constructs by reading from stream. @param stream the stream to read
        VBentoColor(const VString& name, const VColor& c) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(c) {} ///< Constructs from supplied name and value.
        virtual ~VBentoColor() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoColor(this->getName(), mValue); }
        VBentoColor& operator=(const VBentoColor& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual void getValueAsXMLText(VString& s) const { s = mValue.getCSSColor(); }
        virtual void getValueAsString(VString& s) const { s.format("%d,%d,%d,%d", mValue.getRed(), mValue.getGreen(), mValue.getBlue(), mValue.getAlpha()); }
        virtual void getValueAsBentoTextString(VString& s) const { s.format("%d,%d,%d,%d", mValue.getRed(), mValue.getGreen(), mValue.getBlue(), mValue.getAlpha()); }

        inline const VColor& getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the value object
        inline void setValue(const VColor& i) { mValue = i; } ///< Sets the attribute's value. @param i the attribute value

    protected:

        virtual Vs64 getDataLength() const { return 4; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { mValue.writeToStream(stream); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

    private:

        VColor mValue; ///< The attribute value.
    };

/**
VBentoBinary is a VBentoAttribute that holds untyped data.
*/
class VBentoBinary : public VBentoAttribute
    {
    public:
    
        static VBentoBinary* newFromBentoTextString(const VString& name, const VString& bentoText);

        static const VString& DATA_TYPE_ID() { static const VString kID("bina"); return kID; } ///< The data type name / class ID string.

        VBentoBinary() : mValue(0) {} ///< Constructs with uninitialized name and a zero-length buffer.
        VBentoBinary(VBinaryIOStream& stream) : VBentoAttribute(stream, DATA_TYPE_ID()), mValue(0) { Vs64 length = VBentoNode::_readLengthFromStream(stream); (void) VStream::streamCopy(stream, mValue, length); } ///< Constructs by reading from stream. @param stream the stream to read
        VBentoBinary(const VString& name, const Vu8* data, Vs64 length) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(0) { (void) mValue.write(data, length); } ///< Constructs from supplied name and data that is copied.
        VBentoBinary(const VString& name, Vu8* data, VMemoryStream::BufferAllocationType allocationType, bool adoptBuffer, Vs64 suppliedBufferSize, Vs64 suppliedEOFOffset) : VBentoAttribute(name, DATA_TYPE_ID()), mValue(data, allocationType, adoptBuffer, suppliedBufferSize, suppliedEOFOffset) {} ///< Constructs from supplied name and information.
        virtual ~VBentoBinary() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoBinary(this->getName(), mValue.getBuffer(), mValue.getEOFOffset()); }
        VBentoBinary& operator=(const VBentoBinary& rhs) { VBentoAttribute::operator=(rhs); mValue = rhs.mValue; return *this; }

        virtual void getValueAsXMLText(VString& s) const { this->_getValueAsHexString(s); }
        virtual void getValueAsString(VString& s) const { this->_getValueAsHexString(s); }
        virtual void getValueAsBentoTextString(VString& s) const { this->_getValueAsHexString(s); }

        inline VReadOnlyMemoryStream getReader() const { return VReadOnlyMemoryStream(mValue.getBuffer(), mValue.getEOFOffset()); } ///< Returns a read-only memory stream that can be used to read the binary data stream without actually modifying this attribute object; the buffer is not copied @return readable stream on the buffer
        inline void setValue(Vu8* buffer, VMemoryStream::BufferAllocationType allocationType, bool adoptBuffer, Vs64 suppliedBufferSize, Vs64 suppliedEOFOffset) { mValue.adoptBuffer(buffer, allocationType, adoptBuffer, suppliedBufferSize, suppliedEOFOffset); } ///< Sets the attribute's data using VMemoryStream::adoptBuffer(); you can choose to share or copy the data depending on the parameters you specify

    protected:

        virtual Vs64 getDataLength() const { Vs64 bufferLength = mValue.getEOFOffset(); return VBentoNode::_getLengthOfLength(bufferLength) + bufferLength; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const; ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

    private:
    
        void _getValueAsHexString(VString& s) const;

        VMemoryStream mValue; ///< The binary data.
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

        static const VString& DATA_TYPE_ID() { static const VString kID("unkn"); return kID; } ///< The data type name / class ID string.

        VBentoUnknownValue() : mValue() {} ///< Constructs with uninitialized name and empty stream.
        VBentoUnknownValue(VBinaryIOStream& stream, Vs64 dataLength, const VString& dataType); ///< Constructs by reading from stream. @param stream the stream to read @param dataLength the length of stream data to read @param dataType the original data type value
        virtual ~VBentoUnknownValue() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { throw VUnimplementedException("VBentoUnknownValue does not support clone()."); }
        VBentoAttribute& operator=(const VBentoAttribute& /*rhs*/) { throw VUnimplementedException("VBentoUnknownValue does not support operator=()."); }

        virtual void getValueAsXMLText(VString& s) const { VHex::bufferToHexString(mValue.getBuffer(), mValue.getEOFOffset(), s, true/* want leading "0x" */); }
        virtual void getValueAsString(VString& s) const { VHex::bufferToHexString(mValue.getBuffer(), mValue.getEOFOffset(), s, true/* want leading "0x" */); }
        virtual void getValueAsBentoTextString(VString& s) const { VHex::bufferToHexString(mValue.getBuffer(), mValue.getEOFOffset(), s, true/* want leading "0x" */); }

        inline const VMemoryStream& getValue() const { return mValue; } ///< Returns the attribute's value. @return a reference to the unknown-typed data stream

    protected:

        virtual Vs64 getDataLength() const { return mValue.getEOFOffset(); } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const; ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

    private:

        VMemoryStream mValue; ///< The attribute value.
    };

/**
VBentoArray is a VBentoAttribute that holds an array of homogeneous simple values;
there is a subclass for each supported data type.
*/
class VBentoArray : public VBentoAttribute
    {
    public:

        VBentoArray() : VBentoAttribute() {} ///< Constructs with uninitialized name and value.
        VBentoArray(VBinaryIOStream& stream, const VString& dataType) : VBentoAttribute(stream, dataType) {} ///< Constructs by reading from stream. @param stream the stream to read @param dataType the data type of the concrete subclass
        VBentoArray(const VString& name, const VString& dataType) : VBentoAttribute(name, dataType) {} ///< Constructs from supplied name and value.
        virtual ~VBentoArray() {} ///< Destructor.

        VBentoArray& operator=(const VBentoArray& rhs) { VBentoAttribute::operator=(rhs); return *this; }

        virtual bool xmlAppearsAsArray() const { return true; } // Complex attribute requires its own child tag, formatted via writeToXMLTextStream().
        virtual void writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int depth) const = 0; // Force subclasses to implement.

        virtual void getValueAsXMLText(VString&) const {} // n/a, since xmlAppearsAsArray() returns true for this class
        virtual void getValueAsString(VString& s) const { this->_getValueAsBentoTextString(s); }
        virtual void getValueAsBentoTextString(VString& s) const { this->_getValueAsBentoTextString(s); }

    protected:

        virtual int _getNumElements() const = 0;
        virtual void _appendElementBentoText(int elementIndex, VString& s) const = 0;

    private:
    
        void _getValueAsBentoTextString(VString& s) const;
    };

/**
VBentoS8Array is a VBentoArray that holds an array of Vs8 values.
*/
class VBentoS8Array : public VBentoArray
    {
    public:

        static VBentoS8Array* newFromBentoTextString(const VString& name, const VString& bentoText);

        static const VString& DATA_TYPE_ID() { static const VString kID("s8_a"); return kID; } ///< The data type name / class ID string.

        VBentoS8Array() : VBentoArray(), mValue() {} ///< Constructs with uninitialized name and an initially empty array.
        VBentoS8Array(VBinaryIOStream& stream) : VBentoArray(stream, DATA_TYPE_ID()), mValue() { int numElements = static_cast<int>(stream.readS32()); for (int i = 0; i < numElements; ++i) mValue.push_back(stream.readS8()); } ///< Constructs by reading from stream. @param stream the stream to read
        VBentoS8Array(const VString& name) : VBentoArray(name, DATA_TYPE_ID()), mValue() {} ///< Constructs from supplied name, with an initially empty array.
        VBentoS8Array(const VString& name, const Vs8Array& elements) : VBentoArray(name, DATA_TYPE_ID()), mValue(elements) {} ///< Constructs from supplied name and array to be copied.
        virtual ~VBentoS8Array() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoS8Array(this->getName(), mValue); }
        VBentoS8Array& operator=(const VBentoS8Array& rhs) { VBentoArray::operator=(rhs); mValue = rhs.mValue; return *this; }

        inline const Vs8Array& getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(const Vs8Array& elements) { mValue = elements; } ///< Sets the attribute's value. @param elements the vector of elements
        inline void appendValue(Vs8 element) { mValue.push_back(element); } ///< Appends to the attribute's value. @param element the element to append
        inline void appendValues(const Vs8Array& elements) { mValue.insert(mValue.end(), elements.begin(), elements.end()); } ///< Appends to the attribute's value. @param elements the vector of elements

        virtual void writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int depth) const; ///< Override to form this complex attribute as a child tag with its own attributes.

    protected:

        virtual Vs64 getDataLength() const { return 4 + (1 * mValue.size()); } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { int numElements = static_cast<int>(mValue.size()); stream.writeS32(numElements); for (Vs8Array::const_iterator i = mValue.begin(); i != mValue.end(); ++i) stream.writeS8(*i); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

        virtual int _getNumElements() const { return static_cast<int>(mValue.size()); }
        virtual void _appendElementBentoText(int elementIndex, VString& s) const { s += mValue[elementIndex]; }

    private:

        Vs8Array mValue; ///< The attribute value.
    };

/**
VBentoS16Array is a VBentoArray that holds an array of Vs16 values.
*/
class VBentoS16Array : public VBentoArray
    {
    public:

        static VBentoS16Array* newFromBentoTextString(const VString& name, const VString& bentoText);

        static const VString& DATA_TYPE_ID() { static const VString kID("s16a"); return kID; } ///< The data type name / class ID string.

        VBentoS16Array() : VBentoArray(), mValue() {} ///< Constructs with uninitialized name and an initially empty array.
        VBentoS16Array(VBinaryIOStream& stream) : VBentoArray(stream, DATA_TYPE_ID()), mValue() { int numElements = static_cast<int>(stream.readS32()); for (int i = 0; i < numElements; ++i) mValue.push_back(stream.readS16()); } ///< Constructs by reading from stream. @param stream the stream to read
        VBentoS16Array(const VString& name) : VBentoArray(name, DATA_TYPE_ID()), mValue() {} ///< Constructs from supplied name, with an initially empty array.
        VBentoS16Array(const VString& name, const Vs16Array& elements) : VBentoArray(name, DATA_TYPE_ID()), mValue(elements) {} ///< Constructs from supplied name and array to be copied.
        virtual ~VBentoS16Array() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoS16Array(this->getName(), mValue); }
        VBentoS16Array& operator=(const VBentoS16Array& rhs) { VBentoArray::operator=(rhs); mValue = rhs.mValue; return *this; }

        inline const Vs16Array& getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(const Vs16Array& elements) { mValue = elements; } ///< Sets the attribute's value. @param elements the vector of elements
        inline void appendValue(Vs16 element) { mValue.push_back(element); } ///< Appends to the attribute's value. @param element the element to append
        inline void appendValues(const Vs16Array& elements) { mValue.insert(mValue.end(), elements.begin(), elements.end()); } ///< Appends to the attribute's value. @param elements the vector of elements

        virtual void writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int depth) const; ///< Override to form this complex attribute as a child tag with its own attributes.

    protected:

        virtual Vs64 getDataLength() const { return 4 + (2 * mValue.size()); } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { int numElements = static_cast<int>(mValue.size()); stream.writeS32(numElements); for (Vs16Array::const_iterator i = mValue.begin(); i != mValue.end(); ++i) stream.writeS16(*i); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

        virtual int _getNumElements() const { return static_cast<int>(mValue.size()); }
        virtual void _appendElementBentoText(int elementIndex, VString& s) const { s += mValue[elementIndex]; }

    private:

        Vs16Array mValue; ///< The attribute value.
    };

/**
VBentoS32Array is a VBentoArray that holds an array of Vs32 values.
*/
class VBentoS32Array : public VBentoArray
    {
    public:

        static VBentoS32Array* newFromBentoTextString(const VString& name, const VString& bentoText);

        static const VString& DATA_TYPE_ID() { static const VString kID("s32a"); return kID; } ///< The data type name / class ID string.

        VBentoS32Array() : VBentoArray(), mValue() {} ///< Constructs with uninitialized name and an initially empty array.
        VBentoS32Array(VBinaryIOStream& stream) : VBentoArray(stream, DATA_TYPE_ID()), mValue() { int numElements = static_cast<int>(stream.readS32()); for (int i = 0; i < numElements; ++i) mValue.push_back(stream.readS32()); } ///< Constructs by reading from stream. @param stream the stream to read
        VBentoS32Array(const VString& name) : VBentoArray(name, DATA_TYPE_ID()), mValue() {} ///< Constructs from supplied name, with an initially empty array.
        VBentoS32Array(const VString& name, const Vs32Array& elements) : VBentoArray(name, DATA_TYPE_ID()), mValue(elements) {} ///< Constructs from supplied name and array to be copied.
        virtual ~VBentoS32Array() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoS32Array(this->getName(), mValue); }
        VBentoS32Array& operator=(const VBentoS32Array& rhs) { VBentoArray::operator=(rhs); mValue = rhs.mValue; return *this; }

        inline const Vs32Array& getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(const Vs32Array& elements) { mValue = elements; } ///< Sets the attribute's value. @param elements the vector of elements
        inline void appendValue(Vs32 element) { mValue.push_back(element); } ///< Appends to the attribute's value. @param element the element to append
        inline void appendValues(const Vs32Array& elements) { mValue.insert(mValue.end(), elements.begin(), elements.end()); } ///< Appends to the attribute's value. @param elements the vector of elements

        virtual void writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int depth) const; ///< Override to form this complex attribute as a child tag with its own attributes.

    protected:

        virtual Vs64 getDataLength() const { return 4 + (4 * mValue.size()); } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { int numElements = static_cast<int>(mValue.size()); stream.writeS32(numElements); for (Vs32Array::const_iterator i = mValue.begin(); i != mValue.end(); ++i) stream.writeS32(*i); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

        virtual int _getNumElements() const { return static_cast<int>(mValue.size()); }
        virtual void _appendElementBentoText(int elementIndex, VString& s) const { s += mValue[elementIndex]; }

    private:

        Vs32Array mValue; ///< The attribute value.
    };

/**
VBentoS64Array is a VBentoArray that holds an array of Vs64 values.
*/
class VBentoS64Array : public VBentoArray
    {
    public:

        static VBentoS64Array* newFromBentoTextString(const VString& name, const VString& bentoText);

        static const VString& DATA_TYPE_ID() { static const VString kID("s64a"); return kID; } ///< The data type name / class ID string.

        VBentoS64Array() : VBentoArray(), mValue() {} ///< Constructs with uninitialized name and an initially empty array.
        VBentoS64Array(VBinaryIOStream& stream) : VBentoArray(stream, DATA_TYPE_ID()), mValue() { int numElements = static_cast<int>(stream.readS32()); for (int i = 0; i < numElements; ++i) mValue.push_back(stream.readS64()); } ///< Constructs by reading from stream. @param stream the stream to read
        VBentoS64Array(const VString& name) : VBentoArray(name, DATA_TYPE_ID()), mValue() {} ///< Constructs from supplied name, with an initially empty array.
        VBentoS64Array(const VString& name, const Vs64Array& elements) : VBentoArray(name, DATA_TYPE_ID()), mValue(elements) {} ///< Constructs from supplied name and array to be copied.
        virtual ~VBentoS64Array() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoS64Array(this->getName(), mValue); }
        VBentoS64Array& operator=(const VBentoS64Array& rhs) { VBentoArray::operator=(rhs); mValue = rhs.mValue; return *this; }

        inline const Vs64Array& getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(const Vs64Array& elements) { mValue = elements; } ///< Sets the attribute's value. @param elements the vector of elements
        inline void appendValue(Vs64 element) { mValue.push_back(element); } ///< Appends to the attribute's value. @param element the element to append
        inline void appendValues(const Vs64Array& elements) { mValue.insert(mValue.end(), elements.begin(), elements.end()); } ///< Appends to the attribute's value. @param elements the vector of elements

        virtual void writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int depth) const; ///< Override to form this complex attribute as a child tag with its own attributes.

    protected:

        virtual Vs64 getDataLength() const { return 4 + (8 * mValue.size()); } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { int numElements = static_cast<int>(mValue.size()); stream.writeS32(numElements); for (Vs64Array::const_iterator i = mValue.begin(); i != mValue.end(); ++i) stream.writeS64(*i); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

        virtual int _getNumElements() const { return static_cast<int>(mValue.size()); }
        virtual void _appendElementBentoText(int elementIndex, VString& s) const { s += mValue[elementIndex]; }

    private:

        Vs64Array mValue; ///< The attribute value.
    };

/**
VBentoStringArray is a VBentoArray that holds an array of VString values.
*/
class VBentoStringArray : public VBentoArray
    {
    public:

        static VBentoStringArray* newFromBentoTextString(const VString& name, const VString& bentoText);

        static const VString& DATA_TYPE_ID() { static const VString kID("vsta"); return kID; } ///< The data type name / class ID string.

        VBentoStringArray() : VBentoArray(), mValue() {} ///< Constructs with uninitialized name and an initially empty array.
        VBentoStringArray(VBinaryIOStream& stream) : VBentoArray(stream, DATA_TYPE_ID()), mValue() { int numElements = static_cast<int>(stream.readS32()); for (int i = 0; i < numElements; ++i) mValue.push_back(stream.readString()); } ///< Constructs by reading from stream. @param stream the stream to read
        VBentoStringArray(const VString& name) : VBentoArray(name, DATA_TYPE_ID()), mValue() {} ///< Constructs from supplied name, with an initially empty array.
        VBentoStringArray(const VString& name, const VStringVector& elements) : VBentoArray(name, DATA_TYPE_ID()), mValue(elements) {} ///< Constructs from supplied name and array to be copied.
        virtual ~VBentoStringArray() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoStringArray(this->getName(), mValue); }
        VBentoStringArray& operator=(const VBentoStringArray& rhs) { VBentoArray::operator=(rhs); mValue = rhs.mValue; return *this; }

        inline const VStringVector& getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(const VStringVector& elements) { mValue = elements; } ///< Sets the attribute's value. @param elements the vector of elements
        inline void appendValue(const VString& element) { mValue.push_back(element); } ///< Appends to the attribute's value. @param element the element to append
        inline void appendValues(const VStringVector& elements) { mValue.insert(mValue.end(), elements.begin(), elements.end()); } ///< Appends to the attribute's value. @param elements the vector of elements

        virtual void writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int depth) const; ///< Override to form this complex attribute as a child tag with its own attributes.

    protected:

        virtual Vs64 getDataLength() const { Vs64 binaryStringsLength = 0; for (VStringVector::const_iterator i = mValue.begin(); i != mValue.end(); ++i) binaryStringsLength += VBentoNode::_getBinaryStringLength(*i); return 4 + binaryStringsLength; } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { int numElements = static_cast<int>(mValue.size()); stream.writeS32(numElements); for (VStringVector::const_iterator i = mValue.begin(); i != mValue.end(); ++i) stream.writeString(*i); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

        virtual int _getNumElements() const { return static_cast<int>(mValue.size()); }
        virtual void _appendElementBentoText(int elementIndex, VString& s) const { VString valueString = mValue[elementIndex]; valueString.replace("\"","\\\\\""); s += '"'; s += valueString; s += '"'; }

    private:

        VStringVector mValue; ///< The attribute value.
    };

/**
VBentoBoolArray is a VBentoArray that holds an array of bool values.
*/
class VBentoBoolArray : public VBentoArray
    {
    public:

        static VBentoBoolArray* newFromBentoTextString(const VString& name, const VString& bentoText);

        static const VString& DATA_TYPE_ID() { static const VString kID("booa"); return kID; } ///< The data type name / class ID string.

        VBentoBoolArray() : VBentoArray(), mValue() {} ///< Constructs with uninitialized name and an initially empty array.
        VBentoBoolArray(VBinaryIOStream& stream) : VBentoArray(stream, DATA_TYPE_ID()), mValue() { int numElements = static_cast<int>(stream.readS32()); for (int i = 0; i < numElements; ++i) mValue.push_back(stream.readBool()); } ///< Constructs by reading from stream. @param stream the stream to read
        VBentoBoolArray(const VString& name) : VBentoArray(name, DATA_TYPE_ID()), mValue() {} ///< Constructs from supplied name, with an initially empty array.
        VBentoBoolArray(const VString& name, const VBoolArray& elements) : VBentoArray(name, DATA_TYPE_ID()), mValue(elements) {} ///< Constructs from supplied name and array to be copied.
        virtual ~VBentoBoolArray() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoBoolArray(this->getName(), mValue); }
        VBentoBoolArray& operator=(const VBentoBoolArray& rhs) { VBentoArray::operator=(rhs); mValue = rhs.mValue; return *this; }

        inline const VBoolArray& getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(const VBoolArray& elements) { mValue = elements; } ///< Sets the attribute's value. @param elements the vector of elements
        inline void appendValue(bool element) { mValue.push_back(element); } ///< Appends to the attribute's value. @param element the element to append
        inline void appendValues(const VBoolArray& elements) { mValue.insert(mValue.end(), elements.begin(), elements.end()); } ///< Appends to the attribute's value. @param elements the vector of elements

        virtual void writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int depth) const; ///< Override to form this complex attribute as a child tag with its own attributes.

    protected:

        virtual Vs64 getDataLength() const { return 4 + (1 * mValue.size()); } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { int numElements = static_cast<int>(mValue.size()); stream.writeS32(numElements); for (VBoolArray::const_iterator i = mValue.begin(); i != mValue.end(); ++i) stream.writeBool(*i); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

        virtual int _getNumElements() const { return static_cast<int>(mValue.size()); }
        virtual void _appendElementBentoText(int elementIndex, VString& s) const { s += (mValue[elementIndex] ? "true":"false"); }

    private:

        VBoolArray mValue; ///< The attribute value.
    };

/**
VBentoDoubleArray is a VBentoArray that holds an array of VDouble values.
*/
class VBentoDoubleArray : public VBentoArray
    {
    public:

        static VBentoDoubleArray* newFromBentoTextString(const VString& name, const VString& bentoText);

        static const VString& DATA_TYPE_ID() { static const VString kID("duba"); return kID; } ///< The data type name / class ID string.

        VBentoDoubleArray() : VBentoArray(), mValue() {} ///< Constructs with uninitialized name and an initially empty array.
        VBentoDoubleArray(VBinaryIOStream& stream) : VBentoArray(stream, DATA_TYPE_ID()), mValue() { int numElements = static_cast<int>(stream.readS32()); for (int i = 0; i < numElements; ++i) mValue.push_back(stream.readDouble()); } ///< Constructs by reading from stream. @param stream the stream to read
        VBentoDoubleArray(const VString& name) : VBentoArray(name, DATA_TYPE_ID()), mValue() {} ///< Constructs from supplied name, with an initially empty array.
        VBentoDoubleArray(const VString& name, const VDoubleArray& elements) : VBentoArray(name, DATA_TYPE_ID()), mValue(elements) {} ///< Constructs from supplied name and array to be copied.
        virtual ~VBentoDoubleArray() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoDoubleArray(this->getName(), mValue); }
        VBentoDoubleArray& operator=(const VBentoDoubleArray& rhs) { VBentoArray::operator=(rhs); mValue = rhs.mValue; return *this; }

        inline const VDoubleArray& getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(const VDoubleArray& elements) { mValue = elements; } ///< Sets the attribute's value. @param elements the vector of elements
        inline void appendValue(VDouble element) { mValue.push_back(element); } ///< Appends to the attribute's value. @param element the element to append
        inline void appendValues(const VDoubleArray& elements) { mValue.insert(mValue.end(), elements.begin(), elements.end()); } ///< Appends to the attribute's value. @param elements the vector of elements

        virtual void writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int depth) const; ///< Override to form this complex attribute as a child tag with its own attributes.

    protected:

        virtual Vs64 getDataLength() const { return 4 + (8 * mValue.size()); } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { int numElements = static_cast<int>(mValue.size()); stream.writeS32(numElements); for (VDoubleArray::const_iterator i = mValue.begin(); i != mValue.end(); ++i) stream.writeDouble(*i); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

        virtual int _getNumElements() const { return static_cast<int>(mValue.size()); }
        virtual void _appendElementBentoText(int elementIndex, VString& s) const { s += mValue[elementIndex]; }

    private:

        VDoubleArray mValue; ///< The attribute value.
    };

/**
VBentoDurationArray is a VBentoArray that holds an array of VDuration values.
*/
class VBentoDurationArray : public VBentoArray
    {
    public:

        static VBentoDurationArray* newFromBentoTextString(const VString& name, const VString& bentoText);

        static const VString& DATA_TYPE_ID() { static const VString kID("draa"); return kID; } ///< The data type name / class ID string.

        VBentoDurationArray() : VBentoArray(), mValue() {} ///< Constructs with uninitialized name and an initially empty array.
        VBentoDurationArray(VBinaryIOStream& stream) : VBentoArray(stream, DATA_TYPE_ID()), mValue() { int numElements = static_cast<int>(stream.readS32()); for (int i = 0; i < numElements; ++i) mValue.push_back(stream.readDuration()); } ///< Constructs by reading from stream. @param stream the stream to read
        VBentoDurationArray(const VString& name) : VBentoArray(name, DATA_TYPE_ID()), mValue() {} ///< Constructs from supplied name, with an initially empty array.
        VBentoDurationArray(const VString& name, const VDurationVector& elements) : VBentoArray(name, DATA_TYPE_ID()), mValue(elements) {} ///< Constructs from supplied name and array to be copied.
        virtual ~VBentoDurationArray() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoDurationArray(this->getName(), mValue); }
        VBentoDurationArray& operator=(const VBentoDurationArray& rhs) { VBentoArray::operator=(rhs); mValue = rhs.mValue; return *this; }

        inline const VDurationVector& getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(const VDurationVector& elements) { mValue = elements; } ///< Sets the attribute's value. @param elements the vector of elements
        inline void appendValue(const VDuration& element) { mValue.push_back(element); } ///< Appends to the attribute's value. @param element the element to append
        inline void appendValues(const VDurationVector& elements) { mValue.insert(mValue.end(), elements.begin(), elements.end()); } ///< Appends to the attribute's value. @param elements the vector of elements

        virtual void writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int depth) const; ///< Override to form this complex attribute as a child tag with its own attributes.

    protected:

        virtual Vs64 getDataLength() const { return 4 + (8 * mValue.size()); } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { int numElements = static_cast<int>(mValue.size()); stream.writeS32(numElements); for (VDurationVector::const_iterator i = mValue.begin(); i != mValue.end(); ++i) stream.writeDuration(*i); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

        virtual int _getNumElements() const { return static_cast<int>(mValue.size()); }
        virtual void _appendElementBentoText(int elementIndex, VString& s) const { s += mValue[elementIndex].getDurationMilliseconds(); s += "ms"; }

    private:

        VDurationVector mValue; ///< The attribute value.
    };

/**
VBentoInstantArray is a VBentoArray that holds an array of VInstant values.
*/
class VBentoInstantArray : public VBentoArray
    {
    public:

        static VBentoInstantArray* newFromBentoTextString(const VString& name, const VString& bentoText);

        static const VString& DATA_TYPE_ID() { static const VString kID("insa"); return kID; } ///< The data type name / class ID string.

        VBentoInstantArray() : VBentoArray(), mValue() {} ///< Constructs with uninitialized name and an initially empty array.
        VBentoInstantArray(VBinaryIOStream& stream) : VBentoArray(stream, DATA_TYPE_ID()), mValue() { int numElements = static_cast<int>(stream.readS32()); for (int i = 0; i < numElements; ++i) mValue.push_back(stream.readInstant()); } ///< Constructs by reading from stream. @param stream the stream to read
        VBentoInstantArray(const VString& name) : VBentoArray(name, DATA_TYPE_ID()), mValue() {} ///< Constructs from supplied name, with an initially empty array.
        VBentoInstantArray(const VString& name, const VInstantVector& elements) : VBentoArray(name, DATA_TYPE_ID()), mValue(elements) {} ///< Constructs from supplied name and array to be copied.
        virtual ~VBentoInstantArray() {} ///< Destructor.

        virtual VBentoAttribute* clone() const { return new VBentoInstantArray(this->getName(), mValue); }
        VBentoInstantArray& operator=(const VBentoInstantArray& rhs) { VBentoArray::operator=(rhs); mValue = rhs.mValue; return *this; }

        inline const VInstantVector& getValue() const { return mValue; } ///< Returns the attribute's value. @return the value
        inline void setValue(const VInstantVector& elements) { mValue = elements; } ///< Sets the attribute's value. @param elements the vector of elements
        inline void appendValue(const VInstant& element) { mValue.push_back(element); } ///< Appends to the attribute's value. @param element the element to append
        inline void appendValues(const VInstantVector& elements) { mValue.insert(mValue.end(), elements.begin(), elements.end()); } ///< Appends to the attribute's value. @param elements the vector of elements

        virtual void writeToXMLTextStream(VTextIOStream& stream, bool lineWrap, int depth) const; ///< Override to form this complex attribute as a child tag with its own attributes.

    protected:

        virtual Vs64 getDataLength() const { return 4 + (8 * mValue.size()); } ///< Returns the length of this object's raw data only. @return the length of the object's raw data
        virtual void writeDataToBinaryStream(VBinaryIOStream& stream) const { int numElements = static_cast<int>(mValue.size()); stream.writeS32(numElements); for (VInstantVector::const_iterator i = mValue.begin(); i != mValue.end(); ++i) stream.writeInstant(*i); } ///< Writes the object's raw data only to a binary stream. @param stream the stream to write to

        virtual int _getNumElements() const { return static_cast<int>(mValue.size()); }
        virtual void _appendElementBentoText(int elementIndex, VString& s) const { VString instantString; mValue[elementIndex].getUTCString(instantString); s += instantString; }

    private:

        VInstantVector mValue; ///< The attribute value.
    };

#endif /* vbento_h */
