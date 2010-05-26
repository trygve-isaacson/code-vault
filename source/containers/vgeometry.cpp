/*
Copyright c1997-2008 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.0
http://www.bombaydigital.com/
*/

#include "vgeometry.h"

// VGeometry ---------------------------------------------------------------

bool VGeometry::equal(VDouble a, VDouble b)
    {
    return V_FABS(a - b) < 0.000001;
    }

bool VGeometry::notEqual(VDouble a, VDouble b)
    {
    return V_FABS(a - b) >= 0.000001;
    }

bool VGeometry::equal(int a, int b)
    {
    return a == b;
    }

bool VGeometry::notEqual(int a, int b)
    {
    return a != b;
    }

void VGeometry::writePairToStream(VBinaryIOStream& stream, VDouble item1, VDouble item2)
    {
    stream.writeDouble(item1);
    stream.writeDouble(item2);
    }

void VGeometry::readPairFromStream(VBinaryIOStream& stream, VDouble& item1, VDouble& item2)
    {
    item1 = stream.readDouble();
    item2 = stream.readDouble();
    }

void VGeometry::writePairToStream(VBinaryIOStream& stream, int item1, int item2)
    {
    stream.writeS32(static_cast<Vs32>(item1));
    stream.writeS32(static_cast<Vs32>(item2));
    }

void VGeometry::readPairFromStream(VBinaryIOStream& stream, int& item1, int& item2)
    {
    item1 = static_cast<int>(stream.readS32());
    item2 = static_cast<int>(stream.readS32());
    }

void VGeometry::writeTripletToStream(VBinaryIOStream& stream, VDouble item1, VDouble item2, VDouble item3)
    {
    stream.writeDouble(item1);
    stream.writeDouble(item2);
    stream.writeDouble(item3);
    }

void VGeometry::readTripletFromStream(VBinaryIOStream& stream, VDouble& item1, VDouble& item2, VDouble& item3)
    {
    item1 = stream.readDouble();
    item2 = stream.readDouble();
    item3 = stream.readDouble();
    }

void VGeometry::writeTripletToStream(VBinaryIOStream& stream, int item1, int item2, int item3)
    {
    stream.writeS32(static_cast<Vs32>(item1));
    stream.writeS32(static_cast<Vs32>(item2));
    stream.writeS32(static_cast<Vs32>(item3));
    }

void VGeometry::readTripletFromStream(VBinaryIOStream& stream, int& item1, int& item2, int& item3)
    {
    item1 = static_cast<int>(stream.readS32());
    item2 = static_cast<int>(stream.readS32());
    item3 = static_cast<int>(stream.readS32());
    }

VDouble VGeometry::getDistance(VDouble dx, VDouble dy)
    {
    return sqrt((dx*dx) + (dy*dy));
    }

VDouble VGeometry::getDistance(int dx, int dy)
    {
    return sqrt(static_cast<VDouble>(dx*dx) + static_cast<VDouble>(dy*dy));
    }

