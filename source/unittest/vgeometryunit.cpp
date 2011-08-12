/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.2
http://www.bombaydigital.com/
*/

/** @file */

#include "vgeometryunit.h"
#include "vgeometry.h"
#include "vmemorystream.h"

VGeometryUnit::VGeometryUnit(bool logOnSuccess, bool throwOnError) :
VUnit("VGeometryUnit", logOnSuccess, throwOnError)
    {
    }

void VGeometryUnit::run()
    {
    this->_testVSize();
    this->_testVPoint();
    this->_testVPoint3D();
    this->_testVLine();
    this->_testVRect();
    this->_testVPolygon();
    }

void VGeometryUnit::_testVSize()
    {
    // VSize basic tests.
    
    VSize s;
    this->test(VSize::equal(s, VSize(0.0, 0.0)), "size default constructor");

    s.setWidth(1.1);
    s.setHeight(2.2);
    this->test(VSize::equal(s, VSize(1.1, 2.2)), "size setters");
    
    s.rWidth() = 3.3;
    s.rHeight() = 4.4;
    this->test(VSize::equal(s, VSize(3.3, 4.4)), "size set via r/w accessor");
    
    s.rWidth() += 2.2;
    s.rHeight() += 2.2;
    this->test(VSize::equal(s, VSize(5.5, 6.6)), "size incremented via r/w accessor");
    
    const VSize s2(3.3, 4.4);
    this->test(VSize::equal(s2, VSize(3.3, 4.4)), "size parameterized constructor");
    
    s += s2;
    this->test(VSize::equal(s, VSize(8.8, 11.0)), "size operator+=");
    
    s -= s2;
    this->test(VSize::equal(s, VSize(5.5, 6.6)), "size operator-=");
    
    s *= 10.0;
    this->test(VSize::equal(s, VSize(55.0, 66.0)), "size operator*=");
    
    s /= 11.0;
    this->test(VSize::equal(s, VSize(5.0, 6.0)), "size operator/=");
    
    VSize s3;
    
    s3 = s + s2;
    this->test(VSize::equal(s3, VSize(8.3, 10.4)), "size operator+");
    
    s3 = s - s2;
    this->test(VSize::equal(s3, VSize(1.7, 1.6)), "size operator-");
    
    s3 = s * 3.0;
    this->test(VSize::equal(s3, VSize(15.0, 18.0)), "size operator* post");
    
    s3 = 2.0 * s;
    this->test(VSize::equal(s3, VSize(10.0, 12)), "size operator* pre");
    
    s3 = s / 2.0;
    this->test(VSize::equal(s3, VSize(2.5, 3.0)), "size operator/");
    
    try
        {
        s3 = s / 0.0;
        this->test(false, "size failed to throw exception for operator/ divide by zero");
        }
    catch (const VRangeException& /*ex*/)
        {
        this->test(true, "size exception thrown for operator/ divide by zero");
        }

    VSize x1a(100.0, 100.0);
    VSize x1b(100.0, 100.0);
    VSize x2a(100.000001,  100.000001);
    VSize x2b(100.0000011, 100.0000011);
    this->test(x1a == x1b, "size operator==");
    this->test(x1a != x2a, "size operator!=");
    this->test(x2a != x2b, "size operator!=");
    this->test(VSize::equal(x1a, x2a), "size ::equal close enough");
    this->test(VSize::notEqual(x1a, x2b), "size ::notEqual not close enough");
    }

void VGeometryUnit::_testVPoint()
    {
    // VPoint basic tests.
    
    VPoint p;
    this->test(VPoint::equal(p, VPoint(0.0, 0.0)), "point default constructor");

    p.setX(1.1);
    p.setY(2.2);
    this->test(VPoint::equal(p, VPoint(1.1, 2.2)), "point setters");
    
    p.rX() = 3.3;
    p.rY() = 4.4;
    this->test(VPoint::equal(p, VPoint(3.3, 4.4)), "point set via r/w accessor");
    
    p.rX() += 2.2;
    p.rY() += 2.2;
    this->test(VPoint::equal(p, VPoint(5.5, 6.6)), "point incremented via r/w accessor");
    
    const VPoint p2(3.3, 4.4);
    this->test(VPoint::equal(p2, VPoint(3.3, 4.4)), "point parameterized constructor");
    
    p += p2;
    this->test(VPoint::equal(p, VPoint(8.8, 11.0)), "point operator+=");
    
    p -= p2;
    this->test(VPoint::equal(p, VPoint(5.5, 6.6)), "point operator-=");
    
    p *= 10.0;
    this->test(VPoint::equal(p, VPoint(55.0, 66.0)), "point operator*=");
    
    p /= 11.0;
    this->test(VPoint::equal(p, VPoint(5.0, 6.0)), "point operator/=");
    
    VPoint p3;
    
    p3 = p + p2;
    this->test(VPoint::equal(p3, VPoint(8.3, 10.4)), "point operator+");
    
    p3 = p - p2;
    this->test(VPoint::equal(p3, VPoint(1.7, 1.6)), "point operator-");
    
    p3 = p * 3.0;
    this->test(VPoint::equal(p3, VPoint(15.0, 18.0)), "point operator* post");
    
    p3 = 2.0 * p;
    this->test(VPoint::equal(p3, VPoint(10.0, 12)), "point operator* pre");
    
    p3 = p / 2.0;
    this->test(VPoint::equal(p3, VPoint(2.5, 3.0)), "point operator/");
    
    try
        {
        p3 = p / 0.0;
        this->test(false, "point failed to throw exception for operator/ divide by zero");
        }
    catch (const VRangeException& /*ex*/)
        {
        this->test(true, "point exception thrown for operator/ divide by zero");
        }

    VPoint x1a(100.0, 100.0);
    VPoint x1b(100.0, 100.0);
    VPoint x2a(100.000001,  100.000001);
    VPoint x2b(100.0000011, 100.0000011);
    this->test(x1a == x1b, "point operator==");
    this->test(x1a != x2a, "point operator!=");
    this->test(x2a != x2b, "point operator!=");
    this->test(VPoint::equal(x1a, x2a), "point ::equal close enough");
    this->test(VPoint::notEqual(x1a, x2b), "point ::notEqual not close enough");
    }

void VGeometryUnit::_testVPoint3D()
    {
    // VPoint3D basic tests.
    
    VPoint3D p;
    this->test(VPoint3D::equal(p, VPoint3D(0.0, 0.0, 0.0)), "point3D default constructor");

    p.setX(1.1);
    p.setY(2.2);
    p.setZ(3.3);
    this->test(VPoint3D::equal(p, VPoint3D(1.1, 2.2, 3.3)), "point3D setters");
    
    p.rX() = 3.3;
    p.rY() = 4.4;
    p.rZ() = 5.5;
    this->test(VPoint3D::equal(p, VPoint3D(3.3, 4.4, 5.5)), "point3D set via r/w accessor");
    
    p.rX() += 2.2;
    p.rY() += 2.2;
    p.rZ() += 2.2;
    this->test(VPoint3D::equal(p, VPoint3D(5.5, 6.6, 7.7)), "point3D incremented via r/w accessor");
    
    const VPoint3D p2(3.3, 4.4, 5.5);
    this->test(VPoint3D::equal(p2, VPoint3D(3.3, 4.4, 5.5)), "point3D parameterized constructor");
    
    p += p2;
    this->test(VPoint3D::equal(p, VPoint3D(8.8, 11.0, 13.2)), "point3D operator+=");
    
    p -= p2;
    this->test(VPoint3D::equal(p, VPoint3D(5.5, 6.6, 7.7)), "point3D operator-=");
    
    p *= 10.0;
    this->test(VPoint3D::equal(p, VPoint3D(55.0, 66.0, 77.0)), "point3D operator*=");
    
    p /= 11.0;
    this->test(VPoint3D::equal(p, VPoint3D(5.0, 6.0, 7.0)), "point3D operator/=");
    
    VPoint3D p3;
    
    p3 = p + p2;
    this->test(VPoint3D::equal(p3, VPoint3D(8.3, 10.4, 12.5)), "point3D operator+");
    
    p3 = p - p2;
    this->test(VPoint3D::equal(p3, VPoint3D(1.7, 1.6, 1.5)), "point3D operator-");
    
    p3 = p * 3.0;
    this->test(VPoint3D::equal(p3, VPoint3D(15.0, 18.0, 21.0)), "point3D operator* post");
    
    p3 = 2.0 * p;
    this->test(VPoint3D::equal(p3, VPoint3D(10.0, 12.0, 14.0)), "point3D operator* pre");
    
    p3 = p / 2.0;
    this->test(VPoint3D::equal(p3, VPoint3D(2.5, 3.0, 3.5)), "point3D operator/");
    
    try
        {
        p3 = p / 0.0;
        this->test(false, "point3D failed to throw exception for operator/ divide by zero");
        }
    catch (const VRangeException& /*ex*/)
        {
        this->test(true, "point3D exception thrown for operator/ divide by zero");
        }

    VPoint3D x1a(100.0, 100.0, 100.0);
    VPoint3D x1b(100.0, 100.0, 100.0);
    VPoint3D x2a(100.000001,  100.000001,  100.000001);
    VPoint3D x2b(100.0000011, 100.0000011, 100.0000011);
    this->test(x1a == x1b, "point3D operator==");
    this->test(x1a != x2a, "point3D operator!=");
    this->test(x2a != x2b, "point3D operator!=");
    this->test(VPoint3D::equal(x1a, x2a), "point3D ::equal close enough");
    this->test(VPoint3D::notEqual(x1a, x2b), "point3D ::notEqual not close enough");
    }

void VGeometryUnit::_testVLine()
    {
    // VLine basic tests.
    // Avoiding variable name "l" because it looks like "I" or "1".
    // Using v as in vector instead.
    
    VLine v;
    this->test(VLine::equal(v, VLine(VPoint(), VPoint())), "line default constructor");

    v.setP1(VPoint(1.1, 1.1));
    v.setP2(VPoint(2.2, 2.2));
    this->test(VLine::equal(v, VLine(VPoint(1.1, 1.1), VPoint(2.2, 2.2))), "line setters");
    
    VUNIT_ASSERT_EQUAL(v.getDX(), 1.1);
    VUNIT_ASSERT_EQUAL(v.getDY(), 1.1);
    this->test(v.getSize() == VSize(1.1, 1.1), "line size");
    VUNIT_ASSERT_EQUAL(v.getLength(), sqrt((1.1*1.1)+(1.1*1.1)));
    
    v.setPoints(VPoint(1.2, 3.4), VPoint(5.6, 7.8));
    this->test(VLine::equal(v, VLine(VPoint(1.2, 3.4), VPoint(5.6, 7.8))), "setPoints");
    
    v.rP1() = VPoint(3.3, 3.3);
    v.rP2() = VPoint(4.4, 4.4);
    this->test(VLine::equal(v, VLine(VPoint(3.3, 3.3), VPoint(4.4, 4.4))), "line points set via r/w accessor");
    
    v.rP1() += VPoint(2.2, 2.2);
    v.rP2() += VPoint(2.2, 2.2);
    this->test(VLine::equal(v, VLine(VPoint(5.5, 5.5), VPoint(6.6, 6.6))), "line points incremented via r/w accessor");
    
    const VLine v2(VPoint(3.3, 3.3), VPoint(4.4, 4.4));
    this->test(VLine::equal(v2, VLine(VPoint(3.3, 3.3), VPoint(4.4, 4.4))), "line parameterized constructor");
    
    VPoint delta(3.3, 4.4);
    v.translate(delta);
    this->test(VLine::equal(v, VLine(VPoint(8.8, 9.9), VPoint(9.9, 11.0))), "line translate+");
    
    v.translate(-delta);
    this->test(VLine::equal(v, VLine(VPoint(5.5, 5.5), VPoint(6.6, 6.6))), "line translate-");
    
    v.translate(3.3, 4.4);
    this->test(VLine::equal(v, VLine(VPoint(8.8, 9.9), VPoint(9.9, 11.0))), "line translate+ xy");
    
    v.translate(-3.3, -4.4);
    this->test(VLine::equal(v, VLine(VPoint(5.5, 5.5), VPoint(6.6, 6.6))), "line translate- xy");
    
    VLine v3 = v.translated(delta);
    this->test(VLine::equal(v3, VLine(VPoint(8.8, 9.9), VPoint(9.9, 11.0))), "line translated");
    v3 = v.translated(3.3, 4.4);
    this->test(VLine::equal(v3, VLine(VPoint(8.8, 9.9), VPoint(9.9, 11.0))), "line translated xy");

    // Test line normalization behavior.
    // A horizontal line.
    VLine vXpos(VPoint(4.0, 2.0), VPoint(7.0, 2.0));
    VLine vXneg(VPoint(7.0, 2.0), VPoint(4.0, 2.0));
    this->test(VLine::equal(vXpos, vXneg.reversed()), "+x reversed");
    this->test(VLine::equal(vXpos, vXpos.normalized()), "+x normalized");
    this->test(VLine::equal(vXpos, vXneg.normalized()), "-x normalized");
    this->test(VLine::notEqual(vXpos, vXneg), "+x != -x");
    this->test(VLine::same(vXpos, vXneg), "+x same -x");
    // A vertical line.
    VLine vYpos(VPoint(5.0, 2.0), VPoint(5.0, 6.0));
    VLine vYneg(VPoint(5.0, 6.0), VPoint(5.0, 2.0));
    this->test(VLine::equal(vYpos, vYneg.reversed()), "+y reversed");
    this->test(VLine::equal(vYpos, vYpos.normalized()), "+y normalized");
    this->test(VLine::equal(vYpos, vYneg.normalized()), "-y normalized");
    this->test(VLine::notEqual(vYpos, vYneg), "+y != -y");
    this->test(VLine::same(vYpos, vYneg), "+y same -y");
    // A line pointing down and to the right.
    VLine vXYpos(VPoint(1.0, 1.0), VPoint(7.0, 7.0));
    VLine vXYneg(VPoint(7.0, 7.0), VPoint(1.0, 1.0));
    this->test(VLine::equal(vXYpos, vXYneg.reversed()), "+xy reversed");
    this->test(VLine::equal(vXYpos, vXYpos.normalized()), "+xy normalized");
    this->test(VLine::equal(vXYpos, vXYneg.normalized()), "-xy normalized");
    this->test(VLine::notEqual(vXYpos, vXYneg), "+y != -y");
    this->test(VLine::same(vXYpos, vXYneg), "+y same -y");
    // A line pointing up and to the right.
    VLine vXposYneg(VPoint(-2.0, 2.0), VPoint(9.0, -9.0));
    VLine vXnegYpos(VPoint(9.0, -9.0), VPoint(-2.0, 2.0));
    this->test(VLine::equal(vXposYneg, vXnegYpos.reversed()), "+x-y reversed");
    this->test(VLine::equal(vXposYneg, vXposYneg.normalized()), "+x-y normalized");
    this->test(VLine::equal(vXposYneg, vXnegYpos.normalized()), "-x+y normalized");
    this->test(VLine::notEqual(vXposYneg, vXnegYpos), "+x-y != -x+y");
    this->test(VLine::same(vXposYneg, vXnegYpos), "+x-y same -x+y");
    
    // Test unit vectors in each direction and a couple of diagonals.
    VLine unitVector;
    
    v.setPoints(VPoint(), VPoint(7.4, 0.0)); 
    unitVector = v.getUnitVector();
    VUNIT_ASSERT_EQUAL(unitVector.getLength(), 1.0);
    this->test(VLine::equal(unitVector, VLine(VPoint(), VPoint(1.0, 0.0))), "unit vector +x");

    v.setPoints(VPoint(), VPoint(-13.8, 0.0)); 
    unitVector = v.getUnitVector();
    VUNIT_ASSERT_EQUAL(unitVector.getLength(), 1.0);
    this->test(VLine::equal(unitVector, VLine(VPoint(), VPoint(-1.0, 0.0))), "unit vector -x");

    v.setPoints(VPoint(), VPoint(0.0, 12.3)); 
    unitVector = v.getUnitVector();
    VUNIT_ASSERT_EQUAL(unitVector.getLength(), 1.0);
    this->test(VLine::equal(unitVector, VLine(VPoint(), VPoint(0.0, 1.0))), "unit vector +y");

    v.setPoints(VPoint(), VPoint(0.0, -17.4)); 
    unitVector = v.getUnitVector();
    VUNIT_ASSERT_EQUAL(unitVector.getLength(), 1.0);
    this->test(VLine::equal(unitVector, VLine(VPoint(), VPoint(0.0, -1.0))), "unit vector -y");

    VLine line1a(100.0,       100.0,       100.0,       100.0);
    VLine line1b(100.0,       100.0,       100.0,       100.0);
    VLine line2a(100.000001,  100.000001,  100.000001,  100.000001);
    VLine line2b(100.0000011, 100.0000011, 100.0000011, 100.0000011);
    this->test(line1a == line1b, "line operator==");
    this->test(line1a != line2a, "line operator!=");
    this->test(line2a != line2b, "line operator!=");
    this->test(VLine::equal(line1a, line2a), "line ::equal close enough");
    this->test(VLine::notEqual(line1a, line2b), "line ::notEqual not close enough");
    
    // Test the distance calculations.
    VLine   longerLine(VPoint(3.0, 0.0), VPoint(0.0, 3.0));
    VLine   shorterLine(VPoint(3.0, 0.0), VPoint(2.0, 1.0));
    VPoint  p_Nearest1(1.5, 1.5);
    VPoint  p_Nearest2(2.0, 1.0);
    
    VDouble distance;
    VPoint  nearest;
    
    // Test distance calculation we'll use later.
    VDouble expectedDistance1 = sqrt((1.5*1.5) + (1.5*1.5));
    VDouble distanceToNearest1 = VPoint::getDistance(VPoint(), p_Nearest1);
    VUNIT_ASSERT_EQUAL_LABELED(expectedDistance1, distanceToNearest1, "distance between points 1");
    VDouble expectedDistance2 = sqrt(5.0); // 2* + 1*
    VDouble distanceToNearest2 = VPoint::getDistance(VPoint(), p_Nearest2);
    VUNIT_ASSERT_EQUAL_LABELED(expectedDistance2, distanceToNearest2, "distance between points 2");
    
    // Test distance from (0,0) to a line segment where the nearest point is in the segment.
    distance = longerLine.getDistanceToPoint(VPoint(), true);
    VUNIT_ASSERT_EQUAL_LABELED(distance, distanceToNearest1, "distance to interior line segment");
    distance = longerLine.getDistanceToPoint(VPoint(), false);
    VUNIT_ASSERT_EQUAL_LABELED(distance, distanceToNearest1, "distance to interior line");
    nearest = longerLine.getNearestPoint(VPoint(), true);
    this->test(VPoint::equal(nearest, p_Nearest1), "nearest point calculation to interior line segment");
    nearest = longerLine.getNearestPoint(VPoint(), false);
    this->test(VPoint::equal(nearest, p_Nearest1), "nearest point calculation to interior line");
    
    // Test distance from (0,0) to a line segment where the nearest point is outside the segment.
    distance = shorterLine.getDistanceToPoint(VPoint(), true);
    VUNIT_ASSERT_EQUAL_LABELED(distance, distanceToNearest2, "distance to exterior line segment");
    distance = shorterLine.getDistanceToPoint(VPoint(), false);
    VUNIT_ASSERT_EQUAL_LABELED(distance, distanceToNearest1, "distance to exterior line");
    nearest = shorterLine.getNearestPoint(VPoint(), true);
    this->test(VPoint::equal(nearest, p_Nearest2), "nearest point calculation to exterior line segment");
    nearest = shorterLine.getNearestPoint(VPoint(), false);
    this->test(VPoint::equal(nearest, p_Nearest1), "nearest point calculation to exterior line");
    }

void VGeometryUnit::_testVRect()
    {
    // VRect basic tests.
    
    VRect r1;
    this->test(VRect::equal(r1, VRect(VPoint(0.0, 0.0), VPoint(0.0, 0.0))), "rect default constructor");
    
    VRect r2(VPoint(1.2, 3.4), VSize(5.6, 7.8));
    this->test(r2.getLeft() == 1.2 &&
                r2.getTop() == 3.4 &&
                r2.getWidth() == 5.6 &&
                r2.getHeight() == 7.8, "rect size constructor");
    
    VRect r3(VPoint(1.2, 3.4), VPoint(5.6, 7.8));
    this->test(VPoint::equal(r3.getLeftTop(), VPoint(1.2, 3.4)) &&
                VSize::equal(r3.getSize(), VSize(4.4, 4.4)), "rect size constructor");
    
    VRect r4(10.1, 12.1, 25.5, 27.5);
    this->test(r4.getLeft() == 10.1 &&
                r4.getTop() == 12.1 &&
                r4.getWidth() == 25.5 &&
                r4.getHeight() == 27.5, "rect elements constructor");

    VRect r;
    r.moveTo(VPoint(2.3, 4.5));
    this->test(r.getLeftTop() == VPoint(2.3, 4.5), "rect move to");
    
    r.setSize(VSize(6.7, 8.9));
    this->test(r.getSize() == VSize(6.7, 8.9), "rect set size");
    
    r.translate(2.4, 6.8);
    this->test(VPoint::equal(r.getLeftTop(), VPoint(4.7, 11.3)), "rect translate dx dy");
    
    r.translate(VPoint(3.5, 7.9));
    this->test(VPoint::equal(r.getLeftTop(), VPoint(8.2, 19.2)), "rect translate vpoint");
    
    r.setWidth(2.4);
    r.setHeight(6.8);
    this->test(r.getSize() == VSize(2.4, 6.8), "rect set width and height");

    VRect r5(VPoint(20.0, 20.0), VPoint(5.0, 5.0));
    VRect r6 = r5.normalized();
    this->test(VPoint::equal(r6.getLeftTop(), VPoint(5.0, 5.0)), "normalized origin");
    this->test(VSize::equal(r6.getSize(), VSize(15.0, 15.0)), "normalized size");
    
    // r6 is normalized to leftTop=(5.0,5.0) rightBottom=(20.0,20.0)
    this->test(r6.contains(VPoint(7.5, 12.4)), "contains");
    this->test(! r6.contains(VPoint(2.5, 12.4)), "not contains left");
    this->test(! r6.contains(VPoint(2.5, 2.4)), "not contains left above");
    this->test(! r6.contains(VPoint(2.5, 22.4)), "not contains left below");
    this->test(! r6.contains(VPoint(22.5, 12.4)), "not contains right");
    this->test(! r6.contains(VPoint(22.5, 2.4)), "not contains right above");
    this->test(! r6.contains(VPoint(22.5, 22.4)), "not contains right below");
    this->test(! r6.contains(VPoint(7.5, 2.4)), "not contains above");
    this->test(! r6.contains(VPoint(7.5, 22.4)), "not contains below");
    
    r6.expandTo(VPoint(25.0, 25.0));
    this->test(VRect::equal(r6, VRect(VPoint(5.0, 5.0), VPoint(25.0, 25.0))), "expand right below");
    r6.expandTo(VPoint(30.0, 17.3));
    this->test(VRect::equal(r6, VRect(VPoint(5.0, 5.0), VPoint(30.0, 25.0))), "expand right");
    r6.expandTo(VPoint(35.0, 4.0));
    this->test(VRect::equal(r6, VRect(VPoint(5.0, 4.0), VPoint(35.0, 25.0))), "expand right above");
    r6.expandTo(VPoint(7.5, 3.0));
    this->test(VRect::equal(r6, VRect(VPoint(5.0, 3.0), VPoint(35.0, 25.0))), "expand above");
    r6.expandTo(VPoint(4.0, 2.0));
    this->test(VRect::equal(r6, VRect(VPoint(4.0, 2.0), VPoint(35.0, 25.0))), "expand above left");
    r6.expandTo(VPoint(3.0, 9.2));
    this->test(VRect::equal(r6, VRect(VPoint(3.0, 2.0), VPoint(35.0, 25.0))), "expand left");
    r6.expandTo(VPoint(2.0, 30.0));
    this->test(VRect::equal(r6, VRect(VPoint(2.0, 2.0), VPoint(35.0, 30.0))), "expand left below");
    r6.expandTo(VPoint(12.0, 40.0));
    this->test(VRect::equal(r6, VRect(VPoint(2.0, 2.0), VPoint(35.0, 40.0))), "expand below");
    r6.expandTo(VPoint(20.0, 20.0));
    this->test(VRect::equal(r6, VRect(VPoint(2.0, 2.0), VPoint(35.0, 40.0))), "expand inside");
    
    VRect r7(VPoint(10.0, 10.0), VPoint(20.0, 20.0));
    VRect r8(VPoint(15.0, 15.0), VPoint(25.0, 25.0));
    VRect r9 = r7.united(r8);
    this->test(VRect::equal(r9, VRect(VPoint(10.0, 10.0), VPoint(25.0, 25.0))), "united");
    
    VRect r10 = r7.intersected(r8);
    this->test(VRect::equal(r10, VRect(VPoint(15.0, 15.0), VPoint(20.0, 20.0))), "intersected");
    VRect r11(VPoint(5.0, 5.0), VPoint(10.0, 10.0));
    VRect r12(VPoint(20.0, 20.0), VPoint(30.0, 30.0));
    VRect r13 = r11.intersected(r12);
    this->test(VRect::equal(r13, VRect()), "not intersected");
    
    // Test the side line getters. Using same() instead of == or equal() means we don't rely on which "direction" a side's line points.
    VRect r14(VPoint(4.0, 2.0), VPoint(7.0, 3.0));
    VLine r14Top = r14.getTopSide();
    this->test(VLine::same(r14Top, VLine(VPoint(4.0, 2.0), VPoint(7.0, 2.0))), "top side");
    VLine r14Right = r14.getRightSide();
    this->test(VLine::same(r14Right, VLine(VPoint(7.0, 2.0), VPoint(7.0, 3.0))), "right side");
    VLine r14Bottom = r14.getBottomSide();
    this->test(VLine::same(r14Bottom, VLine(VPoint(7.0, 3.0), VPoint(4.0, 3.0))), "bottom side");
    VLine r14Left = r14.getLeftSide();
    this->test(VLine::same(r14Left, VLine(VPoint(4.0, 3.0), VPoint(4.0, 2.0))), "left side");

    // Test nearness calculations.
    this->test(VLine::same(r14Top, r14.getNearestSide(VPoint(5.0, 1.0))), "nearest side a");
    this->test(VLine::same(r14Right, r14.getNearestSide(VPoint(8.0, 2.2))), "nearest side b");
    this->test(VLine::same(r14Bottom, r14.getNearestSide(VPoint(6.0, 4.0))), "nearest side c");
    this->test(VLine::same(r14Left, r14.getNearestSide(VPoint(3.0, 2.7))), "nearest side d");
    
    this->test(r14.getNearestVertex(VPoint(5.0, 1.0)) == r14.getLeftTop(), "nearest vertex a");
    this->test(r14.getNearestVertex(VPoint(8.0, 2.2)) == r14.getRightTop(), "nearest vertex b");
    this->test(r14.getNearestVertex(VPoint(6.0, 4.0)) == r14.getRightBottom(), "nearest vertex c");
    this->test(r14.getNearestVertex(VPoint(3.0, 2.7)) == r14.getLeftBottom(), "nearest vertex d");

    VRect x1a(VPoint(100.0, 100.0), VPoint(200.0, 200.0));
    VRect x1b(VPoint(100.0, 100.0), VPoint(200.0, 200.0));
    VRect x2a(VPoint(100.000001, 100.000001), VPoint(200.0, 200.0));
    VRect x2b(VPoint(100.0000011, 100.0000011), VPoint(200.0, 200.0));
    this->test(x1a == x1b, "rect operator==");
    this->test(x1a != x2a, "rect operator!=");
    this->test(x2a != x2b, "rect operator!=");
    this->test(VRect::equal(x1a, x2a), "rect ::equal close enough");
    this->test(VRect::notEqual(x1a, x2b), "rect ::notEqual not close enough");
    }

void VGeometryUnit::_testVPolygon()
    {
    // VPolygon basic tests.
    
    VPolygon p;
    this->test(p.getNumPoints() == 0, "empty polygon");
    
    p.add(VPoint(100.1, 200.2));
    this->test(p.getNumPoints() == 1, "add 1");
    
    p.add(VPoint(200.2, 300.3));
    this->test(p.getNumPoints() == 2, "add 2");
    
    p.add(VPoint(-99.9, 255.5));
    this->test(p.getNumPoints() == 3, "add 3");
    
    VRect bounds = p.getBounds();
    this->test(VRect::equal(bounds, VRect(VPoint(-99.9, 200.2), VPoint(200.2, 300.3))), "bounds");
    
    VPoint point1 = p.getPoint(1);
    this->test(point1 == VPoint(200.2, 300.3), "getPoint");
    
    VPoint point2 = p[2];
    this->test(point2 == VPoint(-99.9, 255.5), "operator[] read");
    p[2] = VPoint(-88.8, 222.2);
    this->test(p[2] == VPoint(-88.8, 222.2), "operator[] write");
    p[2] = point2;
    this->test(p[2] == point2, "operator[] write (restore)");
    
    // Test the "sides".
    this->test(p.getSide(0) == VLine(VPoint(100.1, 200.2), VPoint(200.2, 300.3)), "side 0");
    this->test(p.getSide(1) == VLine(VPoint(200.2, 300.3), VPoint(-99.9, 255.5)), "side 1");
    this->test(p.getSide(2) == VLine(VPoint(-99.9, 255.5), VPoint(100.1, 200.2)), "side 2");
    try {
        VLine v = p.getSide(3);
        this->test(false, "Failed to throw VRangeException for getSide() out of range index 3");
    }
    catch (const VRangeException&) {
        this->test(true, "Threw VRangeException for getSide() out of range index 3");
    }
    
    // Test nearness calculations.
    this->test(p.getNearestSide(VPoint()) == 2, "nearest side a");
    this->test(p.getNearestSide(VPoint(150.0, 200.0)) == 0, "nearest side b");
    this->test(p.getNearestSide(VPoint(100.0, 350.0)) == 1, "nearest side c");
    this->test(p.getNearestSide(VPoint(-100.0, 100.0)) == 2, "nearest side d");
    
    VUNIT_ASSERT_EQUAL_LABELED(p.getNearestVertex(VPoint()), 0, "nearest vertex a");
    VUNIT_ASSERT_EQUAL_LABELED(p.getNearestVertex(VPoint(150.0, 200.0)), 0, "nearest vertex b");
    VUNIT_ASSERT_EQUAL_LABELED(p.getNearestVertex(VPoint(100.0, 350.0)), 1, "nearest vertex c");
    VUNIT_ASSERT_EQUAL_LABELED(p.getNearestVertex(VPoint(-100.0, 100.0)), 2, "nearest vertex d");
    
    // Test getPoint() range checking.
    
    try {
        VPoint outOfRange = p.getPoint(3);
        this->test(false, "Failed to throw VRangeException for getPoint() out of range index 3");
    }
    catch (const VRangeException&) {
        this->test(true, "Threw VRangeException for getPoint() out of range index 3");
    }
    
    try {
        p.setPoint(3, VPoint());
        this->test(false, "Failed to throw VRangeException for setPoint() out of range index 3");
    }
    catch (const VRangeException&) {
        this->test(true, "Threw VRangeException for setPoint() out of range index 3");
    }
    
    // Note that p[3] is also out of range but no range checking is performed for operator[] and so that should crash.
    
    VPolygon pCopy = p;
    this->test(p == pCopy, "assignment equality");
    VPolygon otherPolygon;
    this->test(p != otherPolygon, "size inequality");
    otherPolygon.add(VPoint(100.1, 200.2));
    otherPolygon.add(VPoint(200.2, 300.3));
    otherPolygon.add(VPoint(-99.9, 255.5));
    this->test(p == otherPolygon, "data equality");
    
    otherPolygon.remove(2); // the last point
    this->test(otherPolygon.getNumPoints() == 2, "remove");
    otherPolygon.add(VPoint(-99.9, 255.5001)); // different by .0001 on one coordinate
    this->test(p != otherPolygon, "data inequality");
    
    otherPolygon.eraseAll();
    this->test(otherPolygon.getNumPoints() == 0, "erase all");
    
    VMemoryStream buffer;
    VBinaryIOStream io(buffer);
    p.writeToStream(io);
    io.seek0();
    VPolygon polygonFromStream;
    polygonFromStream.readFromStream(io);
    this->test(p == polygonFromStream, "stream write + read");
    }

