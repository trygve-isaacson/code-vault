/*
Copyright c1997-2005 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vbento.h"

#include "vexception.h"
#include "vbufferedfilestream.h"
#include "vhex.h"

VBentoAttribute::VBentoAttribute()
: mName("uninitialized"), mDataType(VString::kEmptyString)
    {
    }

VBentoAttribute::VBentoAttribute(VBinaryIOStream& stream, const VString& inDataType)
: mName(VString::kEmptyString), mDataType(inDataType)
    {
    stream.readString(mName);
    }

VBentoAttribute::VBentoAttribute(const VString& inName, const VString& inDataType)
: mName(inName), mDataType(inDataType)
    {
    }

VBentoAttribute::~VBentoAttribute()
    {
    }

const VString& VBentoAttribute::name() const
    {
    return mName;
    }

const VString& VBentoAttribute::dataType() const
    {
    return mDataType;
    }

Vs64 VBentoAttribute::calculateBinarySize() const
    {
    Vs64    fixedLength = 8 + this->dataLength();
    Vs64    lengthOfLength = VBentoNode::getLengthOfLength(fixedLength);
    return lengthOfLength + fixedLength;
    }

void VBentoAttribute::writeToStream(VBinaryIOStream& stream) const
    {
    Vs64    totalSize = this->calculateBinarySize();

    VBentoNode::writeLengthToStream(stream, totalSize);
    VBentoNode::writeFourCharCodeToStream(stream, mDataType);
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

void VBentoAttribute::printStreamLayout(VHex& hexDump) const
    {
    Vs64    totalSize = this->calculateBinarySize();
    std::cout << "Attribute '" << mName << "': length= " << totalSize << ", type=" << mDataType << std::endl;

    VMemoryStream    buffer;
    VBinaryIOStream    stream(buffer);
    
    this->writeToStream(stream);

    hexDump.printHex(buffer.getBuffer(), buffer.eofOffset());
    }

VBentoAttribute* VBentoAttribute::newObjectFromStream(VBinaryIOStream& stream)
    {
    Vs64    theDataLength = VBentoNode::readLengthFromStream(stream);
    VString    theDataType;

    VBentoNode::readFourCharCodeFromStream(stream, theDataType);
    
    if (theDataType == VBentoS8::id())
        return new VBentoS8(stream);
    else if (theDataType == VBentoU8::id())
        return new VBentoU8(stream);
    else if (theDataType == VBentoS16::id())
        return new VBentoS16(stream);
    else if (theDataType == VBentoU16::id())
        return new VBentoU16(stream);
    else if (theDataType == VBentoS32::id())
        return new VBentoS32(stream);
    else if (theDataType == VBentoU32::id())
        return new VBentoU32(stream);
    else if (theDataType == VBentoS64::id())
        return new VBentoS64(stream);
    else if (theDataType == VBentoU64::id())
        return new VBentoU64(stream);
    else if (theDataType == VBentoBool::id())
        return new VBentoBool(stream);
    else if (theDataType == VBentoString::id())
        return new VBentoString(stream);
    else
        return new VBentoUnknownValue(stream, theDataLength, theDataType);
    }

VBentoAttribute* VBentoAttribute::newObjectFromStream(VTextIOStream& /*stream*/)
    {
    VString    theName;
    VString    theClassID;
    VString    valueAsText;

    return new VBentoUnknownValue();
    }

// static
Vs64 VBentoNode::readLengthFromStream(VBinaryIOStream& stream)
    {
    return stream.readDynamicCount();
    }

// static
void VBentoNode::writeLengthToStream(VBinaryIOStream& stream, Vs64 length)
    {
    stream.writeDynamicCount(length);
    }

// static
Vs64 VBentoNode::getLengthOfLength(Vs64 length)
    {
    return VBinaryIOStream::getDynamicCountLength(length);
    }

// static
void VBentoNode::readFourCharCodeFromStream(VBinaryIOStream& stream, VString& code)
    {
    code.preflight(4);
    (void) stream.read(reinterpret_cast<Vu8*> (code.buffer()), CONST_S64(4));
    code.postflight(4);
    }

// static
void VBentoNode::writeFourCharCodeToStream(VBinaryIOStream& stream, const VString& code)
    {
    int    codeLength = code.length();

    (void) stream.write(reinterpret_cast<Vu8*> (code.chars()), V_MIN(4, codeLength));
    
    // In case code is less than 4 chars, pad with spaces. Please don't use such codes,
    // it's not efficient!
    for (int i = codeLength; i < 4; ++i)
        stream.writeS8(' ');
    }

VBentoNode::VBentoNode()
: mName("uninitialized")
    {
    }

VBentoNode::VBentoNode(const VString& inName)
: mName(inName)
    {
    }

VBentoNode::VBentoNode(VBinaryIOStream& stream)
    {
    /* unused Vs64    theDataLength = */ (void) VBentoNode::readLengthFromStream(stream);
    Vs32    numAttributes = stream.readS32();
    Vs32    numChildNodes = stream.readS32();

    stream.readString(mName);

    for (int i = 0; i < numAttributes; ++i)
        this->addAttribute(VBentoAttribute::newObjectFromStream(stream));

    for (int i = 0; i < numChildNodes; ++i)
        this->addChildNode(new VBentoNode(stream));
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

const VString& VBentoNode::name() const
    {
    return mName;
    }

Vs64 VBentoNode::calculateBinarySize() const
    {
    Vs64    fixedLength = 12;    // minimum overhead for this node

    VSizeType    numAttributes = mAttributes.size();
    for (VSizeType i = 0; i < numAttributes; ++i)
        fixedLength += mAttributes[i]->calculateBinarySize();
    
    VSizeType    numChildNodes = mChildNodes.size();
    for (VSizeType i = 0; i < numChildNodes; ++i)
        fixedLength += mChildNodes[i]->calculateBinarySize();
    
    Vs64    lengthOfLength = VBentoNode::getLengthOfLength(fixedLength);

    return lengthOfLength + fixedLength;
    }

void VBentoNode::writeToStream(VBinaryIOStream& stream) const
    {
    VSizeType    numAttributes = mAttributes.size();
    VSizeType    numChildNodes = mChildNodes.size();

    Vs64    totalSize = this->calculateBinarySize();
    
    VBentoNode::writeLengthToStream(stream, totalSize);
    stream.writeS32(static_cast<Vs32> (numAttributes));
    stream.writeS32(static_cast<Vs32> (numChildNodes));
    stream.writeString(mName);

    for (VSizeType i = 0; i < numAttributes; ++i)
        mAttributes[i]->writeToStream(stream);
    
    for (VSizeType i = 0; i < numChildNodes; ++i)
        mChildNodes[i]->writeToStream(stream);
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
        s = VString::kEmptyString;

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
        VBufferedFileStream    stdoutStream(stdout);
        VTextIOStream        printStream(stdoutStream, VTextIOStream::kUseUnixLineEndings);
        
        this->writeToStream(printStream);
        
        stdoutStream.flush();
        }
    catch (const VException& ex)
        {
        std::cout << "VBentoNode::printXML unable to print: '" << ex.what() << "'" << std::endl;
        }
    }

void VBentoNode::printStreamLayout()
    {
    try
        {
        VBufferedFileStream    stdoutStream(stdout);
        VTextIOStream        printStream(stdoutStream, VTextIOStream::kUseUnixLineEndings);
        VHex                hexDump(&printStream);
        
        this->printStreamLayout(hexDump);
        
        stdoutStream.flush();
        }
    catch (const VException& ex)
        {
        std::cout << "VBentoNode::printStreamLayout unable to print: '" << ex.what() << "'" << std::endl;
        }
    }

void VBentoNode::printStreamLayout(VHex& hexDump)
    {
    VMemoryStream    buffer;
    VBinaryIOStream    stream(buffer);

    VSizeType    numAttributes = mAttributes.size();
    VSizeType    numChildNodes = mChildNodes.size();

    Vs64    totalSize = this->calculateBinarySize();
    
    VBentoNode::writeLengthToStream(stream, totalSize);
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

void VBentoNode::addAttribute(VBentoAttribute* attribute)
    {
    mAttributes.push_back(attribute);
    }

void VBentoNode::addChildNode(VBentoNode* node)
    {
    mChildNodes.push_back(node);
    }

const VBentoAttributePtrVector& VBentoNode::attributes() const
    {
    return mAttributes;
    }

const VBentoAttribute* VBentoNode::findAttribute(const VString& name, const VString& inDataType) const
    {
    VSizeType    numAttributes = mAttributes.size();

    for (VSizeType i = 0; i < numAttributes; ++i)
        {
        VBentoAttribute*    attribute = mAttributes[i];

        if ((attribute->name() == name) &&
            (attribute->dataType() == inDataType))
            return attribute;
        }
    
    return NULL;
    }

Vs8 VBentoNode::getS8Value(const VString& inName, Vs8 defaultValue) const
    {
    const VBentoS8*    attribute = dynamic_cast<const VBentoS8*> (this->findAttribute(inName, VBentoS8::id()));
    
    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->value();
    }

Vs8 VBentoNode::getS8Value(const VString& inName) const
    {
    const VBentoS8*    attribute = dynamic_cast<const VBentoS8*> (this->findAttribute(inName, VBentoS8::id()));
    
    if (attribute == NULL)
        throw VException("Attribute %s not found.", inName.chars());
    else
        return attribute->value();
    }

Vu8 VBentoNode::getU8Value(const VString& inName, Vu8 defaultValue) const
    {
    const VBentoU8*    attribute = dynamic_cast<const VBentoU8*> (this->findAttribute(inName, VBentoU8::id()));
    
    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->value();
    }

Vu8 VBentoNode::getU8Value(const VString& inName) const
    {
    const VBentoU8*    attribute = dynamic_cast<const VBentoU8*> (this->findAttribute(inName, VBentoU8::id()));
    
    if (attribute == NULL)
        throw VException("Attribute %s not found.", inName.chars());
    else
        return attribute->value();
    }

Vs16 VBentoNode::getS16Value(const VString& inName, Vs16 defaultValue) const
    {
    const VBentoS16*    attribute = dynamic_cast<const VBentoS16*> (this->findAttribute(inName, VBentoS16::id()));
    
    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->value();
    }

Vs16 VBentoNode::getS16Value(const VString& inName) const
    {
    const VBentoS16*    attribute = dynamic_cast<const VBentoS16*> (this->findAttribute(inName, VBentoS16::id()));
    
    if (attribute == NULL)
        throw VException("Attribute %s not found.", inName.chars());
    else
        return attribute->value();
    }

Vu16 VBentoNode::getU16Value(const VString& inName, Vu16 defaultValue) const
    {
    const VBentoU16*    attribute = dynamic_cast<const VBentoU16*> (this->findAttribute(inName, VBentoU16::id()));
    
    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->value();
    }

Vu16 VBentoNode::getU16Value(const VString& inName) const
    {
    const VBentoU16*    attribute = dynamic_cast<const VBentoU16*> (this->findAttribute(inName, VBentoU16::id()));
    
    if (attribute == NULL)
        throw VException("Attribute %s not found.", inName.chars());
    else
        return attribute->value();
    }

Vs32 VBentoNode::getS32Value(const VString& inName, Vs32 defaultValue) const
    {
    const VBentoS32*    attribute = dynamic_cast<const VBentoS32*> (this->findAttribute(inName, VBentoS32::id()));
    
    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->value();
    }

Vs32 VBentoNode::getS32Value(const VString& inName) const
    {
    const VBentoS32*    attribute = dynamic_cast<const VBentoS32*> (this->findAttribute(inName, VBentoS32::id()));
    
    if (attribute == NULL)
        throw VException("Attribute %s not found.", inName.chars());
    else
        return attribute->value();
    }

Vu32 VBentoNode::getU32Value(const VString& inName, Vu32 defaultValue) const
    {
    const VBentoU32*    attribute = dynamic_cast<const VBentoU32*> (this->findAttribute(inName, VBentoU32::id()));
    
    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->value();
    }

Vu32 VBentoNode::getU32Value(const VString& inName) const
    {
    const VBentoU32*    attribute = dynamic_cast<const VBentoU32*> (this->findAttribute(inName, VBentoU32::id()));
    
    if (attribute == NULL)
        throw VException("Attribute %s not found.", inName.chars());
    else
        return attribute->value();
    }

Vs64 VBentoNode::getS64Value(const VString& inName, Vs64 defaultValue) const
    {
    const VBentoS64*    attribute = dynamic_cast<const VBentoS64*> (this->findAttribute(inName, VBentoS64::id()));
    
    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->value();
    }

Vs64 VBentoNode::getS64Value(const VString& inName) const
    {
    const VBentoS64*    attribute = dynamic_cast<const VBentoS64*> (this->findAttribute(inName, VBentoS64::id()));
    
    if (attribute == NULL)
        throw VException("Attribute %s not found.", inName.chars());
    else
        return attribute->value();
    }

Vu64 VBentoNode::getU64Value(const VString& inName, Vu64 defaultValue) const
    {
    const VBentoU64*    attribute = dynamic_cast<const VBentoU64*> (this->findAttribute(inName, VBentoU64::id()));
    
    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->value();
    }

Vu64 VBentoNode::getU64Value(const VString& inName) const
    {
    const VBentoU64*    attribute = dynamic_cast<const VBentoU64*> (this->findAttribute(inName, VBentoU64::id()));
    
    if (attribute == NULL)
        throw VException("Attribute %s not found.", inName.chars());
    else
        return attribute->value();
    }

bool VBentoNode::getBoolValue(const VString& inName, bool defaultValue) const
    {
    const VBentoBool*    attribute = dynamic_cast<const VBentoBool*> (this->findAttribute(inName, VBentoBool::id()));
    
    if (attribute == NULL)
        return defaultValue;
    else
        return attribute->value();
    }

bool VBentoNode::getBoolValue(const VString& inName) const
    {
    const VBentoBool*    attribute = dynamic_cast<const VBentoBool*> (this->findAttribute(inName, VBentoBool::id()));
    
    if (attribute == NULL)
        throw VException("Attribute %s not found.", inName.chars());
    else
        return attribute->value();
    }

VString* VBentoNode::getStringValue(const VString& inName, VString* defaultValue) const
    {
    VString*            s = defaultValue;
    const VBentoString*    attribute = dynamic_cast<const VBentoString*> (this->findAttribute(inName, VBentoString::id()));
    
    if (attribute != NULL)
        {
        if (s == NULL)
            s = new VString(attribute->value());
        else
            *s = attribute->value();
        }
    
    return s;
    }

VString* VBentoNode::getStringValue(const VString& inName) const
    {
    const VBentoString*    attribute = dynamic_cast<const VBentoString*> (this->findAttribute(inName, VBentoString::id()));
    
    if (attribute == NULL)
        throw VException("Attribute %s not found.", inName.chars());
    else
        return new VString(attribute->value());
    }

const VBentoNodePtrVector& VBentoNode::nodes() const
    {
    return mChildNodes;
    }

const VBentoNode* VBentoNode::findNode(const VString& nodeName) const
    {
    VSizeType    numChildren = mChildNodes.size();

    for (VSizeType i = 0; i < numChildren; ++i)
        {
        VBentoNode*    child = mChildNodes[i];

        if (child->name() == nodeName)
            return child;
        }
    
    return NULL;
    }
    
const VBentoNode* VBentoNode::findNode(const VString& nodeName, const VString& attributeName, const VString& inDataType) const
    {
    VSizeType    numChildren = mChildNodes.size();

    for (VSizeType i = 0; i < numChildren; ++i)
        {
        VBentoNode*    child = mChildNodes[i];

        if (child->name() == nodeName)
            {
            if (child->findAttribute(attributeName, inDataType) != NULL)
                return child;
            }
        }
    
    return NULL;
    }

VBentoUnknownValue::VBentoUnknownValue(VBinaryIOStream& stream, Vs64 inDataLength, const VString& inDataType)
: VBentoAttribute(stream, inDataType), mValue(inDataLength)
    {
    VBinaryIOStream    memoryIOStream(mValue);
    
    streamCopy(stream, memoryIOStream, inDataLength);
    }

void VBentoUnknownValue::writeDataToStream(VBinaryIOStream& /*stream*/) const
    {
    /* FIXME: tbd with correct const-ness...
    
    VBinaryIOStream    memoryIOStream(mValue);
    
    memoryIOStream.seek(0, SEEK_SET);
    streamCopy(memoryIOStream, stream, mValue.eofOffset());
    */
    }

VBentoCallbackParser::VBentoCallbackParser(VBinaryIOStream& stream)
    {
    VBentoCallbackParser::processNode(0, stream);
    }

void VBentoCallbackParser::processNode(int depth, VBinaryIOStream& stream)
    {
    Vs64    theDataLength = VBentoNode::readLengthFromStream(stream);
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
    Vs64    theDataLength = VBentoNode::readLengthFromStream(stream);
    VString    type;
    VString    theName;

    VBentoNode::readFourCharCodeFromStream(stream, type);
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

void VBentoCallbackParser::readAttributeData(int /*depth*/, VBinaryIOStream& stream, Vu64 inDataLength)
    {
    stream.skip(static_cast<Vu64> (inDataLength));
    }

