/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

/** @file */

#include "vsettings.h"

#include "vexception.h"
#include "vmemorystream.h"
#include "vtextiostream.h"
#include "vchar.h"

V_STATIC_INIT_TRACE
    
#undef strlen

// VSettingsNode ------------------------------------------------------------------

VSettingsNode::VSettingsNode(VSettingsTag* parent, const VString& name) :
mParent(parent),
mName(name)
    {
    }

VSettingsNode::VSettingsNode(const VSettingsNode& other) :
mParent(other.mParent),
mName(other.mName)
    {
    }

VSettingsNode& VSettingsNode::operator=(const VSettingsNode& other)
    {
    mParent = other.mParent;
    mName = other.mName;
    
    return *this;
    }

const VSettingsNode* VSettingsNode::findNode(const VString& path) const
    {
    if (path.isEmpty())
        return this;

    VString    nextNodeName;
    VString    theRemainder;
    
    VSettings::splitPathFirst(path, nextNodeName, theRemainder);

    if (theRemainder.isEmpty())
        {
        VSettingsAttribute*    attribute = this->_findAttribute(nextNodeName);
        if (attribute != NULL)
            return attribute;
        }

    VSettingsTag*    child = this->_findChildTag(nextNodeName);
    if (child != NULL)
        return child->findNode(theRemainder);
    else
        return NULL;
    }

int VSettingsNode::countNodes(const VString& path) const
    {
    int        result = 0;
    VString    leadingPath;
    VString    lastNode;
    
    VSettings::splitPathLast(path, leadingPath, lastNode);
    
    const VSettingsNode*    parent = this->findNode(leadingPath);
    
    if (parent != NULL)
        result = parent->countNamedChildren(lastNode);
    
    return result;
    }

void VSettingsNode::deleteNode(const VString& path)
    {
    VString    leadingPath;
    VString    lastNode;
    
    VSettings::splitPathLast(path, leadingPath, lastNode);
    
    // FIXME: const_cast is ugly, but this is the only time we find a node and then do a non-const op on it
    VSettingsNode*    parent = const_cast<VSettingsNode*> (this->findNode(leadingPath));
    
    if (parent != NULL)
        parent->deleteNamedChildren(lastNode);
    else if (leadingPath.isEmpty())
        this->deleteNamedChildren(lastNode);
    }

void VSettingsNode::getName(VString& name) const
    {
    name = mName;
    }

void VSettingsNode::getPath(VString& path) const
    {
    if (mParent == NULL)
        path = mName;
    else
        {
        mParent->getPath(path);
        path += kPathDelimiterChar;
        path += mName;
        }
    }

bool VSettingsNode::isNamed(const VString& name) const
    {
    return mName == name;
    }

int VSettingsNode::getInt(const VString& path, int defaultValue) const
    {
    const VSettingsNode*    nodeForPath = this->findNode(path);

    if (nodeForPath != NULL)
        return nodeForPath->getIntValue();
    else
        return defaultValue;
    }

int VSettingsNode::getInt(const VString& path) const
    {
    const VSettingsNode*    nodeForPath = this->findNode(path);

    if (nodeForPath != NULL)
        return nodeForPath->getIntValue();

    this->throwNotFound("Integer", path);
    
    return 0;    // (will never reach this statement because of throw)
    }

bool VSettingsNode::getBoolean(const VString& path, bool defaultValue) const
    {
    const VSettingsNode*    nodeForPath = this->findNode(path);

    if (nodeForPath != NULL)
        return nodeForPath->getBooleanValue();
    else
        return defaultValue;
    }

bool VSettingsNode::getBoolean(const VString& path) const
    {
    const VSettingsNode*    nodeForPath = this->findNode(path);

    if (nodeForPath != NULL)
        return nodeForPath->getBooleanValue();

    this->throwNotFound("Boolean", path);

    return false;    // (will never reach this statement because of throw)
    }

void VSettingsNode::getString(const VString& path, VString& value, const VString& defaultValue) const
    {
    const VSettingsNode*    nodeForPath = this->findNode(path);

    if (nodeForPath != NULL)
        nodeForPath->getStringValue(value);
    else
        value = defaultValue;
    }

void VSettingsNode::getString(const VString& path, VString& value) const
    {
    const VSettingsNode*    nodeForPath = this->findNode(path);

    if (nodeForPath != NULL)
        nodeForPath->getStringValue(value);
    else
        this->throwNotFound("String", path);
    }

bool VSettingsNode::nodeExists(const VString& path) const
    {
    return (this->findNode(path) != NULL);
    }

void VSettingsNode::addIntValue(const VString& path, int value)
    {
    VString    valueString("%d", value);
    this->addStringValue(path, valueString);
    }

void VSettingsNode::addBooleanValue(const VString& path, bool value)
    {
    VString    valueString(value ? "true":"false");
    this->addStringValue(path, valueString);
    }

void VSettingsNode::addStringValue(const VString& path, const VString& value)
    {
    this->add(path, true, value);
    }

void VSettingsNode::addItem(const VString& path)
    {
    VString    dummy;
    this->add(path, false, dummy);
    }

void VSettingsNode::setIntValue(const VString& path, int value)
    {
    VString    valueString("%d", value);
    this->setStringValue(path, valueString);
    }

void VSettingsNode::setBooleanValue(const VString& path, bool value)
    {
    VString    valueString(value ? "true":"false");
    this->setStringValue(path, valueString);
    }

void VSettingsNode::setStringValue(const VString& path, const VString& value)
    {
    VSettingsNode* node = const_cast<VSettingsNode*> (this->findNode(path));

    if (node == NULL)
        this->addStringValue(path, value);
    else
        node->setLiteral(value);
    }

void VSettingsNode::add(const VString& path, bool hasValue, const VString& value)
    {
    VString    nextNodeName;
    VString    theRemainder;
    
    VSettings::splitPathFirst(path, nextNodeName, theRemainder);
    
    /*
    path = a.b: next=a, rem=b -> add child a, add b to it
    path = 
    */
    
    if (theRemainder.isEmpty())
        {
        this->_addLeafValue(nextNodeName, hasValue, value);
        }
    else
        {
        VSettingsTag*    child = this->_findChildTag(nextNodeName);
        if (child == NULL)
            {
            // If there's an attribute, need to move it down as a child tag.
            VSettingsAttribute*    attribute = this->_findAttribute(nextNodeName);
            if (attribute != NULL)
                {
                child = new VSettingsTag(dynamic_cast<VSettingsTag*>(this), nextNodeName);
                this->addChildNode(child);
                
                VString    attributeValue;
                attribute->getStringValue(attributeValue);
                child->addChildNode(new VSettingsCDATA(dynamic_cast<VSettingsTag*>(this), attributeValue));

                this->_removeAttribute(attribute);
                delete attribute;
                }
            }

        if (child == NULL)
            {
            VString    tagName(nextNodeName);

            if (nextNodeName.endsWith(']'))
                {
                int    leftBracketIndex = nextNodeName.indexOf('[');
                nextNodeName.getSubstring(tagName, 0, leftBracketIndex);
                }
                
            child = new VSettingsTag(dynamic_cast<VSettingsTag*>(this), tagName);
            this->addChildNode(child);
            }

        child->add(theRemainder, hasValue, value);
        }
    }

void VSettingsNode::addValue(const VString& path)
    {
    throw VException("VSettingsNode::addValue called for invalid object at '%s'", path.chars());
    }

void VSettingsNode::addChildNode(VSettingsNode* /*node*/)
    {
    VString    thisPath;
    this->getPath(thisPath);

    throw VException("VSettingsNode::addChildNode called for invalid object at '%s'", thisPath.chars());
    }

VSettingsTag* VSettingsNode::getParent()
    {
    return mParent;
    }

void VSettingsNode::_addLeafValue(const VString& name, bool /*hasValue*/, const VString& value)
    {
    VString    thisPath;
    this->getPath(thisPath);

    throw VException("VSettingsNode::_addLeafValue (%s, %s) called for invalid object at '%s'", name.chars(), value.chars(), thisPath.chars());
    }

void VSettingsNode::throwNotFound(const VString& dataKind, const VString& missingTrail) const
    {
    VString    thisPath;
    this->getPath(thisPath);

    throw VException("%s setting '%s' not found starting at path '%s'.", dataKind.chars(), missingTrail.chars(), thisPath.chars());
    }

const char    VSettingsNode::kPathDelimiterChar = '/';

// VSettings ----------------------------------------------------------------------

VSettings::VSettings() :
VSettingsNode(NULL, VString::kEmptyString)
// mNodes constructs to empty
    {
    }

VSettings::VSettings(VTextIOStream& inputStream) :
VSettingsNode(NULL, VString::kEmptyString)
// mNodes constructs to empty
    {
    VSettings::readFromStream(inputStream);
    }

VSettings::~VSettings()
    {
    for (VSizeType i = 0; i < mNodes.size(); ++i)
        delete mNodes[i];
    }

void VSettings::readFromStream(VTextIOStream& inputStream)
    {
    VSettingsXMLParser    parser(inputStream, &mNodes);
    
    parser.parse();
    }

void VSettings::writeToStream(VTextIOStream& outputStream, int indentLevel)
    {
    for (VSizeType i = 0; i < mNodes.size(); ++i)
        mNodes[i]->writeToStream(outputStream, indentLevel);
    }

void VSettings::debugPrint()
    {
    VMemoryStream    memoryStream;
    VTextIOStream    outputStream(memoryStream);
    
    this->writeToStream(outputStream);
    
    std::cout << "Begin Settings:\n";
    
    // Avoid stdout flush problems: print buffer one line at a time.
    char*        buffer = reinterpret_cast<char*> (memoryStream.getBuffer());
    VSizeType    lengthRemaining = strlen(buffer);
    VString    s;
    
    while (lengthRemaining > 0)
        {
        s = VString::kEmptyString;
        
        char    c = *buffer;
        ++buffer;
        --lengthRemaining;

        while ((c != '\n') && (c != '\r') && (lengthRemaining > 0))
            {
            s += c;

            c = *buffer;
            ++buffer;
            --lengthRemaining;
            }

        std::cout << s.chars() << '\n';
        fflush(stdout);
        }

    std::cout << "End Settings\n";
    
    fflush(stdout);
    }

 const VSettingsNode* VSettings::findNode(const VString& path)  const
    {
    VString    nextNodeName;
    VString    theRemainder;
    
    VSettings::splitPathFirst(path, nextNodeName, theRemainder);
    
    VSettingsTag*    child = this->_findChildTag(nextNodeName);
    if (child != NULL)
        return child->findNode(theRemainder);
    else
        return NULL;
    }

int VSettings::countNamedChildren(const VString& name) const
    {
    int        result = 0;
    VString    childName;

    for (VSizeType i = 0; i < mNodes.size(); ++i)
        {
        mNodes[i]->getName(childName);
        if (childName == name)
            ++result;
        }
    
    return result;
    }

const VSettingsNode* VSettings::getNamedChild(const VString& name, int inIndex) const
    {
    int        numFound = 0;
    VString    childName;

    for (VSizeType i = 0; i < mNodes.size(); ++i)
        {
        VSettingsNode*    child = mNodes[i];

        child->getName(childName);

        if (childName == name)
            {
            if (numFound == inIndex)
                return child;
            
            ++numFound;
            }
        }
    
    return NULL;
    }

void VSettings::deleteNamedChildren(const VString& name)
    {
    // Iterate backwards so it's safe to delete while iterating.

    VString    childName;

    for (VSizeType i = mNodes.size(); i > 0 ; --i)
        {
        VSettingsNode*    child = mNodes[i-1];

        child->getName(childName);

        if (childName == name)
            {
            delete child;
            mNodes.erase(mNodes.begin() + i - 1);
            }
        }
    }

int VSettings::getIntValue() const
    {
    throw VException("Tried to get raw int value on top level settings object.");

    //lint -e527 "Unreachable code at token 'return' [MISRA Rule 52]"
    return 0;    // (will never reach this statement because of throw)
    }

bool VSettings::getBooleanValue() const
    {
    throw VException("Tried to get raw boolean value on top level settings object.");

    //lint -e527 "Unreachable code at token 'return' [MISRA Rule 52]"
    return false;    // (will never reach this statement because of throw)
    }

void VSettings::getStringValue(VString& /*value*/) const
    {
    throw VException("Tried to get raw string value on top level settings object.");
    }

void VSettings::addChildNode(VSettingsNode* node)
    {
    mNodes.push_back(node);
    }

// static
int VSettings::stringToInt(const VString& value)
    {
    return ::atoi(value);
    }

// static
bool VSettings::stringToBoolean(const VString& value)
    {
    return (value == "1" ||
        value == "T" ||
        value == "t" ||
        value == "Y" ||
        value == "y" ||
        value == "TRUE" ||
        value == "true" ||
        value == "YES" ||
        value == "yes");
    }

// static
bool VSettings::isPathLeaf(const VString& path)
    {
    return path.indexOf(kPathDelimiterChar) == -1;
    }

// static
void VSettings::splitPathFirst(const VString& path, VString& nextNodeName, VString& outRemainder)
    {
    // This code handles a leaf even though we kind of expect callers to check that first.

    int    dotLocation = path.indexOf(kPathDelimiterChar);
    
    path.getSubstring(nextNodeName, 0, dotLocation);
    
    if (dotLocation == -1)    // no dot found
        outRemainder = VString::kEmptyString;
    else
        path.getSubstring(outRemainder, dotLocation + 1);
    }

// static
void VSettings::splitPathLast(const VString& path, VString& leadingPath, VString& lastNode)
    {
    // This code handles a leaf even though we kind of expect callers to check that first.

    int    dotLocation = path.lastIndexOf(kPathDelimiterChar);
    
    if (dotLocation == -1)    // no dot found
        leadingPath = VString::kEmptyString;
    else
        path.getSubstring(leadingPath, 0, dotLocation);

    path.getSubstring(lastNode, dotLocation + 1);
    }

VSettingsTag* VSettings::_findChildTag(const VString& name) const
    {
    for (VSizeType i = 0; i < mNodes.size(); ++i)
        if (mNodes[i]->isNamed(name))
            return static_cast<VSettingsTag*> (mNodes[i]);
    
    return NULL;
    }

void VSettings::_addLeafValue(const VString& name, bool /*hasValue*/, const VString& value)
    {
    VString    tagName(name);

    if (name.endsWith(']'))
        {
        int    leftBracketIndex = name.indexOf('[');
        name.getSubstring(tagName, 0, leftBracketIndex);
        }
                
    VSettingsTag*    tag = new VSettingsTag(NULL, tagName);
    
    tag->addChildNode(new VSettingsCDATA(tag, value));

    mNodes.push_back(tag);
    }

// VSettingsTag ------------------------------------------------------------------

VSettingsTag::VSettingsTag(VSettingsTag* parent, const VString& name) :
VSettingsNode(parent, name)
// mAttributes constructs to empty
// mChildNodes constructs to empty
    {
    }

VSettingsTag::~VSettingsTag()
    {
    for (VSizeType i = 0; i < mAttributes.size(); ++i)
        delete mAttributes[i];

    for (VSizeType i = 0; i < mChildNodes.size(); ++i)
        delete mChildNodes[i];
    }

void VSettingsTag::writeToStream(VTextIOStream& outputStream, int indentLevel)
    {
    for (int i = 0; i < indentLevel; ++i)
        outputStream.writeString(" ");
    
    VString    beginTag("<%s", mName.chars());
    outputStream.writeString(beginTag);
    
    if (mAttributes.size() > 0)
        {
        // Write each attribute
        for (VSizeType i = 0; i < mAttributes.size(); ++i)
            {
            outputStream.writeString(" ");
            mAttributes[i]->writeToStream(outputStream);
            }
        }

    if (mChildNodes.empty())
        {
        // Just close the tag and we're done.
        outputStream.writeLine(" />");
        }
    else
        {
        // Close the opening tag.
        outputStream.writeLine(">");

        // Write each child node
        for (VSizeType i = 0; i < mChildNodes.size(); ++i)
            {
            mChildNodes[i]->writeToStream(outputStream, indentLevel + 1);
            }
        
        // Write a closing tag.
        for (int i = 0; i < indentLevel; ++i)
            outputStream.writeString(" ");
        
        VString    endTag("</%s>", mName.chars());
        outputStream.writeLine(endTag);
        }

    }

int VSettingsTag::countNamedChildren(const VString& name) const
    {
    int        result = 0;
    VString    childName;

    for (VSizeType i = 0; i < mAttributes.size(); ++i)
        {
        mAttributes[i]->getName(childName);
        if (childName == name)
            ++result;
        }

    for (VSizeType i = 0; i < mChildNodes.size(); ++i)
        {
        mChildNodes[i]->getName(childName);
        if (childName == name)
            ++result;
        }
    
    return result;
    }

const VSettingsNode* VSettingsTag::getNamedChild(const VString& name, int inIndex) const
    {
    int        numFound = 0;
    VString    childName;

    for (VSizeType i = 0; i < mAttributes.size(); ++i)
        {
        VSettingsAttribute*    attribute = mAttributes[i];

        attribute->getName(childName);

        if (childName == name)
            {
            if (numFound == inIndex)
                return attribute;
            
            ++numFound;
            }
        }

    for (VSizeType i = 0; i < mChildNodes.size(); ++i)
        {
        VSettingsNode*    child = mChildNodes[i];

        child->getName(childName);

        if (childName == name)
            {
            if (numFound == inIndex)
                return child;
            
            ++numFound;
            }
        }
    
    return NULL;
    }

void VSettingsTag::deleteNamedChildren(const VString& name)
    {
    // Iterate backwards so it's safe to delete while iterating.

    VString    childName;

    for (VSizeType i = mAttributes.size(); i > 0; --i)
        {
        VSettingsAttribute*    attribute = mAttributes[i-1];

        attribute->getName(childName);

        if (childName == name)
            {
            delete attribute;
            mAttributes.erase(mAttributes.begin() + i - 1);
            }
        }

    for (VSizeType i = mChildNodes.size(); i > 0 ; --i)
        {
        VSettingsNode*    child = mChildNodes[i-1];

        child->getName(childName);

        if (childName == name)
            {
            delete child;
            mChildNodes.erase(mChildNodes.begin() + i - 1);
            }
        }
    }

void VSettingsTag::addAttribute(VSettingsAttribute* attribute)
    {
    mAttributes.push_back(attribute);
    }

void VSettingsTag::addChildNode(VSettingsNode* node)
    {
    mChildNodes.push_back(node);
    }

int VSettingsTag::getIntValue() const
    {
    VSettingsNode*    cdataNode = this->_findChildTag("<cdata>");
    
    if (cdataNode != NULL)
        return cdataNode->getIntValue();

    this->throwNotFound("Integer", "<cdata>");

    //lint -e527 "Unreachable code at token 'return' [MISRA Rule 52]"
    return 0;    // (will never reach this statement because of throw)
    }

bool VSettingsTag::getBooleanValue() const
    {
    VSettingsNode*    cdataNode = this->_findChildTag("<cdata>");
    
    if (cdataNode != NULL)
        return cdataNode->getBooleanValue();

    this->throwNotFound("Boolean", "<cdata>");

    //lint -e527 "Unreachable code at token 'return' [MISRA Rule 52]"
    return false;    // (will never reach this statement because of throw)
    }

void VSettingsTag::getStringValue(VString& value) const
    {
    VSettingsNode*    cdataNode = this->_findChildTag("<cdata>");
    
    if (cdataNode != NULL)
        cdataNode->getStringValue(value);
    else
        this->throwNotFound("String", "<cdata>");
    }

void VSettingsTag::setLiteral(const VString& value)
    {
    VSettingsNode*    cdataNode = this->_findChildTag("<cdata>");
    
    if (cdataNode != NULL)
        cdataNode->setLiteral(value);
    else
        this->throwNotFound("String", "<cdata>");
    }

VSettingsAttribute* VSettingsTag::_findAttribute(const VString& name) const
    {
    for (VSizeType i = 0; i < mAttributes.size(); ++i)
        if (mAttributes[i]->isNamed(name))
            return mAttributes[i];
    
    return NULL;
    }

VSettingsTag* VSettingsTag::_findChildTag(const VString& name) const
    {
    if (name.endsWith(']'))
        {
        int    leftBracketIndex = name.indexOf('[');
        VString    indexString;
        name.getSubstring(indexString, leftBracketIndex+1, name.length() - 1);
        int    theIndex = atoi(indexString);
        
        VString    nameOnly;
        name.getSubstring(nameOnly, 0, leftBracketIndex);
        
        return static_cast<VSettingsTag*> (const_cast<VSettingsNode*> (this->getNamedChild(nameOnly, theIndex)));
        }
    else
        {
        for (VSizeType i = 0; i < mChildNodes.size(); ++i)
            if (mChildNodes[i]->isNamed(name))
                return static_cast<VSettingsTag*> (mChildNodes[i]);
        }
    
    return NULL;
    }

void VSettingsTag::_addLeafValue(const VString& name, bool hasValue, const VString& value)
    {
    if (hasValue)
        this->addAttribute(new VSettingsAttribute(this, name, value));
    else
        this->addAttribute(new VSettingsAttribute(this, name));
    }

void VSettingsTag::_removeAttribute(VSettingsAttribute* attribute)
    {
    for (VSizeType i = 0; i < mAttributes.size(); ++i)
        if (mAttributes[i] == attribute)
            mAttributes.erase(mAttributes.begin() + i);
    }

// VSettingsAttribute ------------------------------------------------------------

VSettingsAttribute::VSettingsAttribute(VSettingsTag* parent, const VString& name, const VString& value) :
VSettingsNode(parent, name),
mHasValue(true),
mValue(value)
    {
    }

VSettingsAttribute::VSettingsAttribute(VSettingsTag* parent, const VString& name) :
VSettingsNode(parent, name),
mHasValue(false)
// mValue constructs to empty string
    {
    }

void VSettingsAttribute::writeToStream(VTextIOStream& outputStream, int /*indentLevel*/)
    {
    if (mHasValue)
        {
        VString    attributeString("%s=\"%s\"", mName.chars(), mValue.chars());
        outputStream.writeString(attributeString);
        }
    else
        {
        VString    attributeString("%s", mName.chars());
        outputStream.writeString(attributeString);
        }
    }

int VSettingsAttribute::getIntValue() const
    {
    return VSettings::stringToInt(mValue);
    }

bool VSettingsAttribute::getBooleanValue() const
    {
    return VSettings::stringToBoolean(mValue);
    }

void VSettingsAttribute::getStringValue(VString& value) const
    {
    value = mValue;
    }

void VSettingsAttribute::setLiteral(const VString& value)
    {
    mHasValue = true;
    mValue = value;
    }

bool VSettingsAttribute::hasValue() const
    {
    return mHasValue;
    }

// VSettingsCDATA ------------------------------------------------------------

VSettingsCDATA::VSettingsCDATA(VSettingsTag* parent, const VString& cdata) :
VSettingsNode(parent, "<cdata>"),
mCDATA(cdata)
    {
    }

void VSettingsCDATA::writeToStream(VTextIOStream& outputStream, int indentLevel)
    {
    if (indentLevel > 1)    // at indent level 1 we're just a top-level item, indenting is detrimental
        {
        for (int i = 0; i < indentLevel; ++i)
            outputStream.writeString(" ");
        }

    outputStream.writeLine(mCDATA);
    }

int VSettingsCDATA::getIntValue() const
    {
    return VSettings::stringToInt(mCDATA);
    }

bool VSettingsCDATA::getBooleanValue() const
    {
    return VSettings::stringToBoolean(mCDATA);
    }

void VSettingsCDATA::getStringValue(VString& value) const
    {
    value = mCDATA;
    }

void VSettingsCDATA::setLiteral(const VString& value)
    {
    mCDATA = value;
    }

// VSettingsXMLParser --------------------------------------------------------

VSettingsXMLParser::VSettingsXMLParser(VTextIOStream& inputStream, VSettingsNodePtrVector* nodes) :
mInputStream(inputStream),
mNodes(nodes),
// mCurrentLine constructs to empty string
mCurrentLineNumber(0),
mCurrentColumnNumber(0),
mParserState(kReady),
// mElement constructs to empty string
mCurrentTag(NULL)
// mPendingAttributeName constructs to empty string
    {
    }

void VSettingsXMLParser::parse()
    {
    bool    done = false;
    VString    line;
    
    mParserState = kReady;

    while (! done)
        {
        try
            {
            mInputStream.readLine(mCurrentLine);
            
            ++mCurrentLineNumber;

            this->parseLine();
            }
        catch (VEOFException& /*ex*/)
            {
            done = true;
            }
        }
    }

void VSettingsXMLParser::parseLine()
    {
    VChar    c;

    mCurrentColumnNumber = 0;

    for (int i = 0; i < mCurrentLine.length(); ++i)
        {
        ++mCurrentColumnNumber;

        c = mCurrentLine[i];
        
        switch (mParserState)
            {
            case kReady:
                if (c.charValue() == '<')
                    {
                    this->emitCDATA();
                    this->changeState(kTag1_open);
                    }
                else
                    this->accumulate(c);
                break;

            case kComment1_bang:
                if (c.charValue() == '-')
                    this->changeState(kComment2_bang_dash);
                else
                    {
                    VString    s("Invalid character '%c' after presumed start of comment.", c.charValue());
                    this->stateError(s);
                    }
                break;

            case kComment2_bang_dash:
                if (c.charValue() == '-')
                    this->changeState(kComment3_in_comment);
                else
                    {
                    VString    s("Invalid character '%c' after presumed start of comment.", c.charValue());
                    this->stateError(s);
                    }
                break;

            case kComment3_in_comment:
                if (c.charValue() == '-')
                    this->changeState(kComment4_traildash);
                else
                    ; // nothing
                break;

            case kComment4_traildash:
                if (c.charValue() == '-')
                    this->changeState(kComment5_traildash_dash);
                else
                    this->changeState(kComment3_in_comment);
                break;

            case kComment5_traildash_dash:
                if (c.charValue() == '-')
                    ; // nothing
                else if (c.charValue() == '>')
                    this->changeState(kReady);
                else
                    this->changeState(kComment3_in_comment);
                break;

            case kTag1_open:
                if (c.charValue() == '!')
                    this->changeState(kComment1_bang);
                else if (c.charValue() == '/')
                    this->changeState(kCloseTag1_open_slash);
                else if (c.isAlpha())
                    {
                    this->changeState(kTag2_in_name);
                    this->accumulate(c);
                    }
                else if (c.isWhitespace())
                    ; // nothing
                else
                    this->stateError("Invalid character after opening tag bracket.");
                break;

            case kTag2_in_name:
                if (VSettingsXMLParser::isValidTagNameChar(c))
                    this->accumulate(c);
                else if (c.isWhitespace())
                    {
                    this->emitOpenTagName();
                    this->changeState(kTag3_post_name);
                    }
                else if (c.charValue() == '/')
                    {
                    this->emitOpenTagName();
                    this->changeState(kTag8_solo_close_slash);
                    }
                else if (c.charValue() == '>')
                    {
                    this->emitOpenTagName();
                    this->changeState(kReady);
                    }
                else
                    {
                    VString    s("Invalid character '%c' in tag name.", c.charValue());
                    this->stateError(s);
                    }
                break;

            case kTag3_post_name:
                if (c.isWhitespace())
                    ; // nothing
                else if (c.charValue() == '>')
                    this->changeState(kReady);
                else if (c.charValue() == '/')
                    this->changeState(kTag8_solo_close_slash);
                else if (c.isAlpha())
                    {
                    this->changeState(kTag4_in_attribute_name);
                    this->accumulate(c);
                    }
                else
                    {
                    VString    s("Invalid character '%c' in tag after name.", c.charValue());
                    this->stateError(s);
                    }
                break;

            case kTag4_in_attribute_name:
                if (VSettingsXMLParser::isValidAttributeNameChar(c))
                    this->accumulate(c);
                else if (c.charValue() == '=')
                    {
                    this->emitAttributeName();
                    this->changeState(kTag5_attribute_equals);
                    }
                else if (c.isWhitespace())
                    {
                    this->emitAttributeNameOnly();
                    this->changeState(kTag3_post_name);
                    }
                else if (c.charValue() == '/')
                    {
                    this->emitAttributeNameOnly();
                    this->changeState(kTag8_solo_close_slash);
                    }
                else
                    {
                    VString    s("Invalid character '%c' in attribute name.", c.charValue());
                    this->stateError(s);
                    }
                break;

            case kTag5_attribute_equals:
                if (c.charValue() == '\"')
                    this->changeState(kTag6_attribute_quoted);
                else if (c.charValue() == '/')
                    {
                    this->emitAttributeValue();
                    this->changeState(kTag8_solo_close_slash);
                    }
                else if (c.isAlphaNumeric())
                    {
                    this->changeState(kTag7_attribute_unquoted);
                    this->accumulate(c);
                    }
                break;

            case kTag6_attribute_quoted:
                if (c.isAlphaNumeric())
                    this->accumulate(c);
                else if (c.isWhitespace())
                    this->accumulate(c);
                else if (c.charValue() == '\"')
                    {
                    this->emitAttributeValue();
                    this->changeState(kTag3_post_name);
                    }
                else
                    this->accumulate(c);
                break;

            case kTag7_attribute_unquoted:
                if (VSettingsXMLParser::isValidAttributeValueChar(c))
                    this->accumulate(c);
                else if (c.isWhitespace())
                    {
                    this->emitAttributeValue();
                    this->changeState(kTag3_post_name);
                    }
                else if (c.charValue() == '>')
                    {
                    this->emitAttributeValue();
                    this->changeState(kReady);
                    }
                else if (c.charValue() == '/')
                    {
                    this->emitAttributeValue();
                    this->changeState(kTag8_solo_close_slash);
                    }
                else
                    {
                    VString    s("Invalid character '%c' in unquoted attribute value.", c.charValue());
                    this->stateError(s);
                    }
                break;

            case kTag8_solo_close_slash:
                if (c.charValue() == '>')
                    {
                    this->emitEndSoloTag();
                    this->changeState(kReady);
                    }
                else
                    {
                    VString    s("Invalid character '%c' after solo close tag slash.", c.charValue());
                    this->stateError(s);
                    }
                break;

            case kCloseTag1_open_slash:
                if (c.isWhitespace())
                    ; // nothing
                else if (VSettingsXMLParser::isValidTagNameChar(c))
                    {
                    this->changeState(kCloseTag2_in_name);
                    this->accumulate(c);
                    }
                else
                    {
                    VString    s("Invalid character '%c' in closing tag.", c.charValue());
                    this->stateError(s);
                    }
                break;

            case kCloseTag2_in_name:
                if (c.charValue() == '>')
                    {
                    this->emitCloseTagName();
                    this->changeState(kReady);
                    }
                else if (c.isWhitespace())
                    {
                    this->emitCloseTagName();
                    this->changeState(kCloseTag3_trailing_whitespace);
                    }
                else if (VSettingsXMLParser::isValidTagNameChar(c))
                    this->accumulate(c);
                else
                    {
                    VString    s("Invalid character '%c' in closing tag.", c.charValue());
                    this->stateError(s);
                    }
                break;

            case kCloseTag3_trailing_whitespace:
                if (c.isWhitespace())
                    ; // nothing
                else if (c.charValue() == '>')
                    this->changeState(kReady);
                else
                    {
                    VString    s("Invalid character '%c' in closing tag.", c.charValue());
                    this->stateError(s);
                    }
                break;
            }
        
        if (c.charValue() == '\t')
            mCurrentColumnNumber += 3;    // already did ++, and we want tabs to be 4 "columns" in terms of syntax errors
        }
    }

void VSettingsXMLParser::resetElement()
    {
    mElement = VString::kEmptyString;
    }

void VSettingsXMLParser::accumulate(const VChar& c)
    {
    mElement += c;
    }

void VSettingsXMLParser::changeState(ParserState newState)
    {
    mParserState = newState;
    this->resetElement();
    }

void VSettingsXMLParser::stateError(const VString& errorMessage)
    {
    VString     completeMessage("Syntax error in state %d at line %d, column %d: %s", mParserState, mCurrentLineNumber, mCurrentColumnNumber, errorMessage.chars());
/*
    std::cout << "Syntax error at line " << mCurrentLineNumber << ", column " << mCurrentColumnNumber << ":" << std::endl;
    std::cout << errorMessage << std::endl;
    std::cout << mCurrentLine << std::endl;
    for (int i = 1; i < mCurrentColumnNumber; ++i)    // col number is 1-based
        std::cout << " ";
    std::cout << "^" << std::endl;    // little arrow under column where syntax error was found
*/

    throw VException(completeMessage);
    }

void VSettingsXMLParser::emitCDATA()
    {
    mElement.trim();

    if (! mElement.isEmpty())
        {
///        std::cout << "CDATA: '" << mElement << "'" << std::endl;
        
        if (mCurrentTag == NULL)
            mNodes->push_back(new VSettingsCDATA(NULL, mElement));
        else
            mCurrentTag->addChildNode(new VSettingsCDATA(mCurrentTag, mElement));
        }
    }

void VSettingsXMLParser::emitOpenTagName()
    {
//    std::cout << "open tag name: '" << mElement << "'" << std::endl;
    
    VSettingsTag*    tag = new VSettingsTag(mCurrentTag, mElement);

    if (mCurrentTag == NULL)
        mNodes->push_back(tag);
    else
        mCurrentTag->addChildNode(tag);

    mCurrentTag = tag;
    }

void VSettingsXMLParser::emitAttributeName()
    {
//    std::cout << "attribute name: '" << mElement << "'" << std::endl;
    
    mPendingAttributeName = mElement;
    }

void VSettingsXMLParser::emitAttributeNameOnly()
    {
//    std::cout << "attribute name only: '" << mElement << "'" << std::endl;

    VSettingsAttribute*    attribute = new VSettingsAttribute(mCurrentTag, mElement);

    mCurrentTag->addAttribute(attribute);
    }

void VSettingsXMLParser::emitAttributeValue()
    {
//    std::cout << "attribute value: '" << mElement << "'" << std::endl;

    VSettingsAttribute*    attribute = new VSettingsAttribute(mCurrentTag, mPendingAttributeName, mElement);

    mCurrentTag->addAttribute(attribute);
    }

void VSettingsXMLParser::emitCloseTagName()
    {
//    std::cout << "close tag name: '" << mElement << "'" << std::endl;
    
    VString    currentTagName;
    mCurrentTag->getName(currentTagName);
    
    if (currentTagName != mElement)
        this->stateError("Closing tag name does not balance opening tag.");
    
    mCurrentTag = mCurrentTag->getParent();
    }

void VSettingsXMLParser::emitEndSoloTag()
    {
//    std::cout << "end solo tag" << std::endl;
    
    mCurrentTag = mCurrentTag->getParent();
    }

// static
bool VSettingsXMLParser::isValidTagNameChar(const VChar& c)
    {
    char    value = c.charValue();

    return ((value > 0x20) && (value < 0x7F) && (value != '<') && (value != '>') && (value != '/') && (value != '='));
    }

// static
bool VSettingsXMLParser::isValidAttributeNameChar(const VChar& c)
    {
    char    value = c.charValue();

    return ((value > 0x20) && (value < 0x7F) && (value != '<') && (value != '>') && (value != '/') && (value != '='));
    }

// static
bool VSettingsXMLParser::isValidAttributeValueChar(const VChar& c)
    {
    char    value = c.charValue();

    return ((value > 0x20) && (value < 0x7F) && (value != '<') && (value != '>') && (value != '/') && (value != '='));
    }

