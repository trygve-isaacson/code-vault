/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

#ifndef vsettings_h
#define vsettings_h

/** @file */

#include "vstring.h"

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
VSettings provides a facility for storing and retrieving settings in an
XML text format (with some restrictions on the actual format for simplicity).

A VSettings object represents the settings file itself, or a particular
node in the file. It is derived from VSettingsNode, which is used internally
for parsing and managing the settings data structures.
*/
class VSettingsNode
    {
    public:
        
        VSettingsNode(VSettingsTag* parent, const VString& name);
        virtual ~VSettingsNode() {}

        virtual void writeToStream(VTextIOStream& outputStream, int indentLevel = 0) = 0;

        virtual const VSettingsNode* findNode(const VString& path) const;
        virtual int countNodes(const VString& path) const;
        virtual int countNamedChildren(const VString& /*name*/) const { return 0; }
        virtual const VSettingsNode* getNamedChild(const VString& /*name*/, int /*inIndex*/) const { return NULL; }
        virtual void deleteNode(const VString& path);
        virtual void deleteNamedChildren(const VString& /*name*/) {}

        virtual void getName(VString& name) const;
        virtual void getPath(VString& path) const;
        virtual bool isNamed(const VString& name) const;

        virtual int getInt(const VString& path, int defaultValue) const;
        virtual int getInt(const VString& path) const;
        virtual int getIntValue() const = 0;
        virtual bool getBoolean(const VString& path, bool defaultValue) const;
        virtual bool getBoolean(const VString& path) const;
        virtual bool getBooleanValue() const = 0;
        virtual void getString(const VString& path, VString& value, const VString& defaultValue) const;
        virtual void getString(const VString& path, VString& value) const;
        virtual void getStringValue(VString& value) const = 0;
        virtual bool nodeExists(const VString& path) const;
        
        virtual void addIntValue(const VString& path, int value);
        virtual void addBooleanValue(const VString& path, bool value);
        virtual void addStringValue(const VString& path, const VString& value);
        virtual void addItem(const VString& path);
        virtual void setIntValue(const VString& path, int value);
        virtual void setBooleanValue(const VString& path, bool value);
        virtual void setStringValue(const VString& path, const VString& value);
        virtual void setLiteral(const VString& /*value*/) {};

        virtual void add(const VString& path, bool hasValue, const VString& value);

        virtual void addValue(const VString& value);

        virtual void addChildNode(VSettingsNode* node);
        
        VSettingsTag* getParent();
    
    protected:

        virtual VSettingsAttribute* findAttribute(const VString& /*name*/) const { return NULL; }
        virtual VSettingsTag* findChildTag(const VString& /*name*/) const { return NULL; }
        virtual void addLeafValue(const VString& name, bool hasValue, const VString& value);
        virtual void removeAttribute(VSettingsAttribute* /*attribute*/) {}
        
        void throwNotFound(const VString& dataKind, const VString& missingTrail) const;
        
        static const char    kPathDelimiterChar;

        VSettingsTag*    mParent;
        VString            mName;
    };

class VSettings : public VSettingsNode
    {
    public:
        
        VSettings();
        VSettings(VTextIOStream& inputStream);
        virtual ~VSettings();
        
        virtual void readFromStream(VTextIOStream& inputStream);
        virtual void writeToStream(VTextIOStream& outputStream, int indentLevel = 0);
        void debugPrint();

        virtual const VSettingsNode* findNode(const VString& path) const;
        virtual int countNamedChildren(const VString& name) const;
        virtual const VSettingsNode* getNamedChild(const VString& name, int inIndex) const;
        virtual void deleteNamedChildren(const VString& name);

        virtual int getIntValue() const;
        virtual bool getBooleanValue() const;
        virtual void getStringValue(VString& value) const;

        virtual void addChildNode(VSettingsNode* node);

        // String value converters.
        static int stringToInt(const VString& value);
        static bool stringToBoolean(const VString& value);
        
        // Path navigation utilities.
        static bool isPathLeaf(const VString& path);
        static void splitPathFirst(const VString& path, VString& nextNodeName, VString& outRemainder);
        static void splitPathLast(const VString& path, VString& leadingPath, VString& lastNode);
        
    protected:
    
        virtual VSettingsTag* findChildTag(const VString& /*name*/) const;
        virtual void addLeafValue(const VString& name, bool hasValue, const VString& value);
        
        VSettingsNodePtrVector    mNodes;
        
    };

class VSettingsTag : public VSettingsNode
    {
    public:
        
        VSettingsTag(VSettingsTag* parent, const VString& name);
        virtual ~VSettingsTag();

        virtual void writeToStream(VTextIOStream& outputStream, int indentLevel = 0);

        virtual int countNamedChildren(const VString& name) const;
        virtual const VSettingsNode* getNamedChild(const VString& name, int inIndex) const;
        virtual void deleteNamedChildren(const VString& name);

        void addAttribute(VSettingsAttribute* attribute);
        virtual void addChildNode(VSettingsNode* node);
        
        virtual int getIntValue() const;
        virtual bool getBooleanValue() const;
        virtual void getStringValue(VString& value) const;

        virtual void setLiteral(const VString& value);

    protected:

        virtual VSettingsAttribute* findAttribute(const VString& name) const;
        virtual VSettingsTag* findChildTag(const VString& name) const;
        virtual void addLeafValue(const VString& name, bool hasValue, const VString& value);
        virtual void removeAttribute(VSettingsAttribute* attribute);

        VSettingsAttributePtrVector    mAttributes;
        VSettingsNodePtrVector        mChildNodes;
    
    };

class VSettingsAttribute : public VSettingsNode
    {
    public:
    
        VSettingsAttribute(VSettingsTag* parent, const VString& name, const VString& value);
        VSettingsAttribute(VSettingsTag* parent, const VString& name);
        virtual ~VSettingsAttribute() {}

        virtual void writeToStream(VTextIOStream& outputStream, int indentLevel = 0);

        virtual int getIntValue() const;
        virtual bool getBooleanValue() const;
        virtual void getStringValue(VString& value) const;

        virtual void setLiteral(const VString& value);
        
        bool hasValue() const;
        
    protected:
    
        bool    mHasValue;
        VString    mValue;
    };

class VSettingsCDATA : public VSettingsNode
    {
    public:
        
        VSettingsCDATA(VSettingsTag* parent, const VString& cdata);
        virtual ~VSettingsCDATA() {}

        virtual void writeToStream(VTextIOStream& outputStream, int indentLevel = 0);

        virtual int getIntValue() const;
        virtual bool getBooleanValue() const;
        virtual void getStringValue(VString& value) const;

        virtual void setLiteral(const VString& value);
        
    protected:
    
        VString    mCDATA;
    };

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
    
        VTextIOStream&            mInputStream;
        VSettingsNodePtrVector*    mNodes;
        VString                    mCurrentLine;
        int                        mCurrentLineNumber;
        int                        mCurrentColumnNumber;
        
        ParserState        mParserState;
        VString    mElement;
    
        VSettingsTag*    mCurrentTag;
        VString            mPendingAttributeName;
        
        static bool isValidTagNameChar(const VChar& c);
        static bool isValidAttributeNameChar(const VChar& c);
        static bool isValidAttributeValueChar(const VChar& c);
        
    };

#endif /* vsettings_h */

