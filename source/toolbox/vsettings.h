/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

#ifndef vsettings_h
#define vsettings_h

/** @file */

#include "vstring.h"
#include "vbento.h"

class VTextIOStream;

/**
    @ingroup toolbox
*/

class VSettingsNode;
typedef std::vector<VSettingsNode*> VSettingsNodePtrVector;

class VSettingsAttribute;
typedef std::vector<VSettingsAttribute*> VSettingsAttributePtrVector;

class VSettingsTag;

/**

    @defgroup settings Vault Settings

    <h3>Vault Streams Architecture</h3>

    VSettings provides a facility for storing and retrieving settings in an
    XML text format (with some restrictions on the actual format for simplicity).

    A VSettings object represents the settings file itself, or a particular
    node in the file. It is derived from VSettingsNode, which is used internally
    for parsing and managing the settings data structures.
*/

/**
VSettingsNode is the abstract class used to describe any node in the settings hierarchy.
It may be the top level VSettings object, an arbitrary tag node, an attribute/value node,
or a CDATA node.
*/
class VSettingsNode
    {
    public:
        
        VSettingsNode(VSettingsTag* parent, const VString& name);
        VSettingsNode(const VSettingsNode& other);
        virtual ~VSettingsNode() {}

        VSettingsNode& operator=(const VSettingsNode& other);

        virtual void writeToStream(VTextIOStream& outputStream, int indentLevel = 0) const = 0;
        virtual VBentoNode* writeToBento() const = 0;    

        virtual const VSettingsNode* findNode(const VString& path) const;
        virtual VSettingsNode* findMutableNode(const VString& path);
        virtual int countNodes(const VString& path) const;
        virtual int countNamedChildren(const VString& /*name*/) const { return 0; }
        virtual const VSettingsNode* getNamedChild(const VString& /*name*/, int /*index*/) const { return NULL; }
        virtual void deleteNode(const VString& path);
        virtual void deleteNamedChildren(const VString& /*name*/) {}

        const VString& getName() const;
        VString getPath() const;
        bool isNamed(const VString& name) const;

        virtual int getInt(const VString& path, int defaultValue) const;
        virtual int getInt(const VString& path) const;
        int getIntValue() const; // Because it all comes from text, int is just cast from getS64() parsed value.
        virtual Vs64 getS64(const VString& path, Vs64 defaultValue) const;
        virtual Vs64 getS64(const VString& path) const;
        virtual Vs64 getS64Value() const = 0;
        virtual bool getBoolean(const VString& path, bool defaultValue) const;
        virtual bool getBoolean(const VString& path) const;
        virtual bool getBooleanValue() const = 0;
        virtual VString getString(const VString& path, const VString& defaultValue) const;
        virtual VString getString(const VString& path) const;
        virtual VString getStringValue() const = 0;
        virtual VDouble getDouble(const VString& path, VDouble defaultValue) const;
        virtual VDouble getDouble(const VString& path) const;
        virtual VDouble getDoubleValue() const = 0;
        virtual VSize getSize(const VString& path, const VSize& defaultValue) const;
        virtual VSize getSize(const VString& path) const;
        virtual VSize getSizeValue() const = 0;
        virtual VPoint getPoint(const VString& path, const VPoint& defaultValue) const;
        virtual VPoint getPoint(const VString& path) const;
        virtual VPoint getPointValue() const = 0;
        virtual VRect getRect(const VString& path, const VRect& defaultValue) const;
        virtual VRect getRect(const VString& path) const;
        virtual VRect getRectValue() const = 0;
        virtual VPolygon getPolygon(const VString& path, const VPolygon& defaultValue) const;
        virtual VPolygon getPolygon(const VString& path) const;
        virtual VPolygon getPolygonValue() const = 0;
        virtual VColor getColor(const VString& path, const VColor& defaultValue) const;
        virtual VColor getColor(const VString& path) const;
        virtual VColor getColorValue() const = 0;
        virtual VDuration getDuration(const VString& path, const VDuration& defaultValue) const;
        virtual VDuration getDuration(const VString& path) const;
        virtual VDuration getDurationValue() const = 0;
        virtual bool nodeExists(const VString& path) const;
        
        virtual void addIntValue(const VString& path, int value);
        virtual void addS64Value(const VString& path, Vs64 value);
        virtual void addBooleanValue(const VString& path, bool value);
        virtual void addStringValue(const VString& path, const VString& value);
        virtual void addDoubleValue(const VString& path, VDouble value);
        virtual void addSizeValue(const VString& path, const VSize& value);
        virtual void addPointValue(const VString& path, const VPoint& value);
        virtual void addRectValue(const VString& path, const VRect& value);
        virtual void addPolygonValue(const VString& path, const VPolygon& value);
        virtual void addColorValue(const VString& path, const VColor& value);
        virtual void addDurationValue(const VString& path, const VDuration& value);
        virtual void addItem(const VString& path);
        virtual void setIntValue(const VString& path, int value);
        virtual void setBooleanValue(const VString& path, bool value);
        virtual void setStringValue(const VString& path, const VString& value);
        virtual void setDoubleValue(const VString& path, VDouble value);
        virtual void setSizeValue(const VString& path, const VSize& value);
        virtual void setPointValue(const VString& path, const VPoint& value);
        virtual void setRectValue(const VString& path, const VRect& value);
        virtual void setPolygonValue(const VString& path, const VPolygon& value);
        virtual void setColorValue(const VString& path, const VColor& value);
        virtual void setDurationValue(const VString& path, const VDuration& value);
        virtual void setLiteral(const VString& /*value*/) {};

        virtual void add(const VString& path, bool hasValue, const VString& value);

        virtual void addValue(const VString& value);

        virtual void addChildNode(VSettingsNode* node);
        
        VSettingsTag* getParent();
    
    protected:

        virtual VSettingsAttribute* _findAttribute(const VString& /*name*/) const { return NULL; }
        virtual VSettingsTag* _findChildTag(const VString& /*name*/) const { return NULL; }
        virtual void _addLeafValue(const VString& name, bool hasValue, const VString& value);
        virtual void _removeAttribute(VSettingsAttribute* /*attribute*/) {}
        virtual void _removeChildNode(VSettingsNode* /*child*/) {}
        
        void throwNotFound(const VString& dataKind, const VString& missingTrail) const;
        
        static const char kPathDelimiterChar;

        VSettingsTag*   mParent;
        VString         mName;
    };

/**
VSettings is the top level object you use to read and write a settings hierarchy. It is derived
from the generic node class.
*/
class VSettings : public VSettingsNode
    {
    public:
        
        VSettings();
        VSettings(const VFSNode& file);
        VSettings(VTextIOStream& inputStream);
        virtual ~VSettings();
        
        void readFromFile(const VFSNode& file);
        void writeToFile(const VFSNode& file) const;
        void readFromStream(VTextIOStream& inputStream);
        virtual void writeToStream(VTextIOStream& outputStream, int indentLevel = 0) const;
        virtual VBentoNode* writeToBento() const;
        void debugPrint() const;

        virtual const VSettingsNode* findNode(const VString& path) const;
        virtual int countNamedChildren(const VString& name) const;
        virtual const VSettingsNode* getNamedChild(const VString& name, int index) const;
        virtual void deleteNamedChildren(const VString& name);

        virtual Vs64 getS64Value() const;
        virtual bool getBooleanValue() const;
        virtual VString getStringValue() const;
        virtual VDouble getDoubleValue() const;
        virtual VSize getSizeValue() const;
        virtual VPoint getPointValue() const;
        virtual VRect getRectValue() const;
        virtual VPolygon getPolygonValue() const;
        virtual VColor getColorValue() const;
        virtual VDuration getDurationValue() const;

        virtual void addChildNode(VSettingsNode* node);

        // String value converters.
        static bool stringToBoolean(const VString& value);
        
        // Path navigation utilities.
        static bool isPathLeaf(const VString& path);
        static void splitPathFirst(const VString& path, VString& nextNodeName, VString& outRemainder);
        static void splitPathLast(const VString& path, VString& leadingPath, VString& lastNode);
        
    protected:
    
        virtual VSettingsTag* _findChildTag(const VString& /*name*/) const;
        virtual void _addLeafValue(const VString& name, bool hasValue, const VString& value);
    
    private:
    
        VSettingsNodePtrVector mNodes;
        
    };

/**
A VSettingsTag node is one that has optional attribute/value pairs and optional child nodes.
*/
class VSettingsTag : public VSettingsNode
    {
    public:
        
        // In reality, the parent can be a VSettings object; but no other kind of VSettings Node (i.e., attributes).
        VSettingsTag(VSettingsTag* parent, const VString& name);
        virtual ~VSettingsTag();

        virtual void writeToStream(VTextIOStream& outputStream, int indentLevel = 0) const;
        virtual VBentoNode* writeToBento() const;

        virtual int countNamedChildren(const VString& name) const;
        virtual const VSettingsNode* getNamedChild(const VString& name, int index) const;
        virtual void deleteNamedChildren(const VString& name);

        void addAttribute(VSettingsAttribute* attribute);
        virtual void addChildNode(VSettingsNode* node);
        
        virtual Vs64 getS64Value() const;
        virtual bool getBooleanValue() const;
        virtual VString getStringValue() const;
        virtual VDouble getDoubleValue() const;
        virtual VSize getSizeValue() const;
        virtual VPoint getPointValue() const;
        virtual VRect getRectValue() const;
        virtual VPolygon getPolygonValue() const;
        virtual VColor getColorValue() const;
        virtual VDuration getDurationValue() const;

        virtual void setLiteral(const VString& value);

    protected:

        virtual VSettingsAttribute* _findAttribute(const VString& name) const;
        virtual VSettingsTag* _findChildTag(const VString& name) const;
        virtual void _addLeafValue(const VString& name, bool hasValue, const VString& value);
        virtual void _removeAttribute(VSettingsAttribute* attribute);
        virtual void _removeChildNode(VSettingsNode* child);

    private:

        VSettingsAttributePtrVector mAttributes;
        VSettingsNodePtrVector      mChildNodes;
    
    };

/**
A VSettingsAttribute is a node that has a name and value. Because all data originates as text
from XML, you can attempt to get the value as the desired type, and if it can be converted it
will be. An exception is thrown if the type is incompatible, for example if you getIntValue()
for the string "this is not an integer".
*/
class VSettingsAttribute : public VSettingsNode
    {
    public:
    
        VSettingsAttribute(VSettingsTag* parent, const VString& name, const VString& value);
        VSettingsAttribute(VSettingsTag* parent, const VString& name);
        virtual ~VSettingsAttribute() {}

        virtual void writeToStream(VTextIOStream& outputStream, int indentLevel = 0) const;
        virtual VBentoNode* writeToBento() const;

        virtual Vs64 getS64Value() const;
        virtual bool getBooleanValue() const;
        virtual VString getStringValue() const;
        virtual VDouble getDoubleValue() const;
        virtual VSize getSizeValue() const;
        virtual VPoint getPointValue() const;
        virtual VRect getRectValue() const;
        virtual VPolygon getPolygonValue() const;
        virtual VColor getColorValue() const;
        virtual VDuration getDurationValue() const;

        virtual void setLiteral(const VString& value);
        
        bool hasValue() const;
        
    private:
    
        bool    mHasValue;
        VString mValue;
    };

/**
VSettingsCDATA is a node that represents text inside the tag hierarchy; what it really means
is an attribute value whose name (as in name/value) is the tag name of its parent. For example,
if you have "<tag1><tag2>hello</tag2></tag1>" then tag2 is represented by a VSettingsTag that
has a VSettingsCDATA child whose CDATA value is "hello". It could retrieved from the root node
via ->getString("tag1/tag2");
*/
class VSettingsCDATA : public VSettingsNode
    {
    public:
        
        VSettingsCDATA(VSettingsTag* parent, const VString& cdata);
        virtual ~VSettingsCDATA() {}

        virtual void writeToStream(VTextIOStream& outputStream, int indentLevel = 0) const;
        virtual VBentoNode* writeToBento() const;

        virtual Vs64 getS64Value() const;
        virtual bool getBooleanValue() const;
        virtual VString getStringValue() const;
        virtual VDouble getDoubleValue() const;
        virtual VSize getSizeValue() const;
        virtual VPoint getPointValue() const;
        virtual VRect getRectValue() const;
        virtual VPolygon getPolygonValue() const;
        virtual VColor getColorValue() const;
        virtual VDuration getDurationValue() const;

        virtual void setLiteral(const VString& value);
        
    private:
    
        VString mCDATA;
    };

/**
This is the class that contains the logic for parsing XML text into VSettings objects.
*/
class VSettingsXMLParser
    {
    public:
    
        VSettingsXMLParser(VTextIOStream& inputStream, VSettingsNodePtrVector* nodes);
        virtual ~VSettingsXMLParser() {}
        
        void parse();

    protected:

        enum ParserState
            {
            kReady,
            kComment1_bang,
            kComment2_bang_dash,
            kComment3_in_comment,
            kComment4_traildash,
            kComment5_traildash_dash,
            kTag1_open,
            kTag2_in_name,
            kTag3_post_name,
            kTag4_in_attribute_name,
            kTag5_attribute_equals,
            kTag6_attribute_quoted,
            kTag7_attribute_unquoted,
            kTag8_solo_close_slash,
            kCloseTag1_open_slash,
            kCloseTag2_in_name,
            kCloseTag3_trailing_whitespace
            };
    
        void parseLine();
        void resetElement();
        void accumulate(const VChar& c);
        void changeState(ParserState newState);
        void stateError(const VString& errorMessage);
        
        void emitCDATA();
        void emitOpenTagName();
        void emitAttributeName();
        void emitAttributeNameOnly();
        void emitAttributeValue();
        void emitCloseTagName();
        void emitEndSoloTag();
    
        VTextIOStream&          mInputStream;
        VSettingsNodePtrVector* mNodes;
        VString                 mCurrentLine;
        int                     mCurrentLineNumber;
        int                     mCurrentColumnNumber;
        ParserState             mParserState;
        VString                 mElement;
        VSettingsTag*           mCurrentTag;
        VString                 mPendingAttributeName;
        
        static bool isValidTagNameChar(const VChar& c);
        static bool isValidAttributeNameChar(const VChar& c);
        static bool isValidAttributeValueChar(const VChar& c);
    
    private:
    
        // Prevent copy construction and assignment since there is no provision for sharing pointer data.
        VSettingsXMLParser(const VSettingsXMLParser& other);
        VSettingsXMLParser& operator=(const VSettingsXMLParser& other);
        
    };

#endif /* vsettings_h */

