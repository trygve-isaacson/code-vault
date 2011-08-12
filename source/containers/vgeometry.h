/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.2
http://www.bombaydigital.com/
*/

#ifndef vgeometry_h
#define vgeometry_h

/** @file */

#include "vtypes.h"

#include "vexception.h"
#include "vbinaryiostream.h"

#ifdef VAULT_QT_SUPPORT
#include <QSize>
#include <QSizeF>
#include <QPoint>
#include <QPointF>
#include <QLine>
#include <QLineF>
#include <QRect>
#include <QRectF>
#include <QPolygon>
#include <QPolygonF>
#endif

namespace VGeometry
    {
    /*
    When using VDouble types, tests for equality and inequality are of dubious use, because
    fine precision can be lost even in the simplest calculations, rendering unexpected results.
    (For example, 5.9 + 2.3 may not "==" 8.2 .)
    The equal() and notEqual() functions let you test for approximate equality within 0.000001.
    In order for the templates to work with both VDouble and int, we define int versions,
    too, and those do an exact test.
    */
    extern bool equal(VDouble a, VDouble b);
    extern bool notEqual(VDouble a, VDouble b);
    extern bool equal(int a, int b);
    extern bool notEqual(int a, int b);
    extern void writePairToStream(VBinaryIOStream& stream, VDouble item1, VDouble item2);
    extern void readPairFromStream(VBinaryIOStream& stream, VDouble& item1, VDouble& item2);
    extern void writePairToStream(VBinaryIOStream& stream, int item1, int item2);
    extern void readPairFromStream(VBinaryIOStream& stream, int& item1, int& item2);
    extern void writeTripletToStream(VBinaryIOStream& stream, VDouble item1, VDouble item2, VDouble item3);
    extern void readTripletFromStream(VBinaryIOStream& stream, VDouble& item1, VDouble& item2, VDouble& item3);
    extern void writeTripletToStream(VBinaryIOStream& stream, int item1, int item2, int item3);
    extern void readTripletFromStream(VBinaryIOStream& stream, int& item1, int& item2, int& item3);
    extern VDouble getDistance(VDouble dx, VDouble dy);
    extern VDouble getDistance(int dx, int dy);
    }

/*
These types are define as templates, simply in order to allow each to be used with
both "int" and "VDouble" coordinates, without requiring complete duplicated implementations.
Because they are templates, all functions must be coded inline here; no separating
the declaration here from an implementation in the .cpp file. Wherever there is a short
function I just put it on the same line as the function name; otherwise I break it out
into a multi-line code block kind of like a normal implementation.
*/

/**
VSizeT defines a width and height. VSize uses VDouble values; VISize uses int values.
*/

template <typename T> class VSizeT;
template <typename T> bool operator== (const VSizeT<T>& s1, const VSizeT<T>& s2);
template <typename T> bool operator!= (const VSizeT<T>& s1, const VSizeT<T>& s2);
template <typename T> VSizeT<T> operator+ (const VSizeT<T>& s1, const VSizeT<T>& s2);
template <typename T> VSizeT<T> operator- (const VSizeT<T>& s1, const VSizeT<T>& s2);
template <typename T> VSizeT<T> operator* (const VSizeT<T>& x, T scale);
template <typename T> VSizeT<T> operator* (T scale, const VSizeT<T>& x);
template <typename T> VSizeT<T> operator/ (const VSizeT<T>& x, T divisor);

template <typename T>
class VSizeT
    {
    public:
    
        VSizeT() : mWidth(0), mHeight(0) {}
        VSizeT(T width, T height) : mWidth(width), mHeight(height) {}
        VSizeT(VBinaryIOStream& stream) : mWidth(0), mHeight(0) { VGeometry::readPairFromStream(stream, mWidth, mHeight); }
        ~VSizeT() {}

#ifdef VAULT_QT_SUPPORT
        VSizeT(const QSize& s) : mWidth(s.width()), mHeight(s.height()) {}
        VSizeT(const QSizeF& s) : mWidth(static_cast<T>(s.width())), mHeight(static_cast<T>(s.height())) {}
#endif
    
        void readFromStream(VBinaryIOStream& stream) { VGeometry::readPairFromStream(stream, mWidth, mHeight); }
        void writeToStream(VBinaryIOStream& stream) const { VGeometry::writePairToStream(stream, mWidth, mHeight); }
        
        T getWidth() const { return mWidth; }
        T getHeight() const { return mHeight; }
        void setWidth(T width) { mWidth = width; }
        void setHeight(T height) { mHeight = height; }
        T& rWidth() { return mWidth; }
        T& rHeight() { return mHeight; }
        
        VSizeT<T>& operator+=(const VSizeT<T>& x) { mWidth += x.mWidth; mHeight += x.mHeight; return *this; }
        VSizeT<T>& operator-=(const VSizeT<T>& x) { mWidth -= x.mWidth; mHeight -= x.mHeight; return *this; }
        VSizeT<T>& operator*=(T scale) { mWidth *= scale; mHeight *= scale; return *this; }
        VSizeT<T>& operator/=(T divisor) { if (divisor == 0) throw VRangeException("VSizeT divide by zero."); mWidth /= divisor; mHeight /= divisor; return *this; }

        // These use "approximate" equality by testing for a difference >= 0.000001.
        static bool equal(const VSizeT<T>& s1, const VSizeT<T>& s2)
            {
            return VGeometry::equal(s1.mWidth, s2.mWidth) && VGeometry::equal(s1.mHeight, s2.mHeight);
            }

        static bool notEqual(const VSizeT<T>& s1, const VSizeT<T>& s2)
            {
            return VGeometry::notEqual(s1.mWidth, s2.mWidth) || VGeometry::notEqual(s1.mHeight, s2.mHeight);
            }

        friend bool operator== <>(const VSizeT<T>& s1, const VSizeT<T>& s2);     // exact equality
        friend bool operator!= <>(const VSizeT<T>& s1, const VSizeT<T>& s2);     // exact inequality
        friend VSizeT<T> operator+ <>(const VSizeT<T>& s1, const VSizeT<T>& s2); // addition
        friend VSizeT<T> operator- <>(const VSizeT<T>& s1, const VSizeT<T>& s2); // subtraction
        friend VSizeT<T> operator* <>(const VSizeT<T>& x, T scale);              // multiplication p*n
        friend VSizeT<T> operator* <>(T scale, const VSizeT<T>& x);              // multiplication n*p
        friend VSizeT<T> operator/ <>(const VSizeT<T>& x, T divisor);            // division

#ifdef VAULT_QT_SUPPORT
        void setQSize(const QSize& s) { mWidth = s.width(); mHeight = s.height(); }
        QSize getQSize() const { return QSize(static_cast<int>(mWidth), static_cast<int>(mHeight)); }
        void setQSizeF(const QSizeF& s) { mWidth = static_cast<T>(s.width()); mHeight = static_cast<T>(s.height()); }
        QSizeF getQSizeF() const { return QSizeF(mWidth, mHeight); }
#endif
        
    private:
    
        T mWidth;
        T mHeight;
    };

template<typename T> bool operator==(const VSizeT<T>& s1, const VSizeT<T>& s2) { return s1.mWidth == s2.mWidth && s1.mHeight == s2.mHeight; }
template<typename T> bool operator!=(const VSizeT<T>& s1, const VSizeT<T>& s2) { return s1.mWidth != s2.mWidth || s1.mHeight != s2.mHeight; }
template<typename T> VSizeT<T> operator+(const VSizeT<T>& s1, const VSizeT<T>& s2) { return VSizeT<T>(s1.mWidth + s2.mWidth, s1.mHeight + s2.mHeight); }
template<typename T> VSizeT<T> operator-(const VSizeT<T>& s1, const VSizeT<T>& s2) { return VSizeT<T>(s1.mWidth - s2.mWidth, s1.mHeight - s2.mHeight); }
template<typename T> VSizeT<T> operator*(const VSizeT<T>& x, T scale) { return VSizeT<T>(x.mWidth * scale, x.mHeight * scale); }
template<typename T> VSizeT<T> operator*(T scale, const VSizeT<T>& x) { return VSizeT<T>(x.mWidth * scale, x.mHeight * scale); }
template<typename T> VSizeT<T> operator/(const VSizeT<T>& x, T divisor) { if (divisor == 0) throw VRangeException("VSizeT divide by zero."); return VSizeT<T>(x.mWidth / divisor, x.mHeight / divisor); }

typedef VSizeT<VDouble> VSize;
typedef VSizeT<int> VISize;

/**
VPointT defines a point with x and y coordinates. VPoint uses VDouble coordinates; VIPoint uses int coordinates.
*/
template <typename T> class VPointT;
template <typename T> bool operator== (const VPointT<T>& p1, const VPointT<T>& p2);
template <typename T> bool operator!= (const VPointT<T>& p1, const VPointT<T>& p2);
template <typename T> VPointT<T> operator+ (const VPointT<T>& p1, const VPointT<T>& p2);
template <typename T> VPointT<T> operator+ (const VPointT<T>& p, const VSizeT<T>& s);
template <typename T> VPointT<T> operator- (const VPointT<T>& p1, const VPointT<T>& p2);
template <typename T> VPointT<T> operator- (const VPointT<T>& p, const VSizeT<T>& s);
template <typename T> VPointT<T> operator- (const VPointT<T>& p);
template <typename T> VPointT<T> operator* (const VPointT<T>& p, T scale);
template <typename T> VPointT<T> operator* (T scale, const VPointT<T>& p);
template <typename T> VPointT<T> operator/ (const VPointT<T>& p, T divisor);

template <typename T>
class VPointT
    {
    public:
    
        /** Returns the distance between two points. */
        static VDouble getDistance(const VPointT<T>& p1, const VPointT<T>& p2) { return VGeometry::getDistance(p2.getX()-p1.getX(), p2.getY()-p1.getY()); }
    
        VPointT() : mX(0), mY(0) {}
        VPointT(T x, T y) : mX(x), mY(y) {}
        VPointT(VBinaryIOStream& stream) : mX(0), mY(0) { VGeometry::readPairFromStream(stream, mX, mY); }
        ~VPointT() {}

#ifdef VAULT_QT_SUPPORT
        VPointT(const QPoint& p) : mX(p.x()), mY(p.y()) {}
        VPointT(const QPointF& p) : mX(static_cast<T>(p.x())), mY(static_cast<T>(p.y())) {}
#endif
            
        void readFromStream(VBinaryIOStream& stream) { VGeometry::readPairFromStream(stream, mX, mY); }
        void writeToStream(VBinaryIOStream& stream) const { VGeometry::writePairToStream(stream, mX, mY); }
        
        T getX() const { return mX; }
        T getY() const { return mY; }
        void setX(T x) { mX = x; }
        void setY(T y) { mY = y; }
        T& rX() { return mX; }
        T& rY() { return mY; }

        VPointT<T>& operator+=(const VPointT<T>& p) { mX += p.mX; mY += p.mY; return *this; }
        VPointT<T>& operator-=(const VPointT<T>& p) { mX -= p.mX; mY -= p.mY; return *this; }
        VPointT<T>& operator*=(T scale) { mX *= scale; mY *= scale; return *this; }
        VPointT<T>& operator/=(T divisor) { if (divisor == 0) throw VRangeException("VPointT divide by zero."); mX /= divisor; mY /= divisor; return *this; }

        // These use "approximate" equality by testing for a difference >= 0.000001.
        static bool equal(const VPointT<T>& p1, const VPointT<T>& p2)
            {
            return VGeometry::equal(p1.mX, p2.mX) && VGeometry::equal(p1.mY, p2.mY);
            }

        static bool notEqual(const VPointT<T>& p1, const VPointT<T>& p2)
            {
            return VGeometry::notEqual(p1.mX, p2.mX) || VGeometry::notEqual(p1.mY, p2.mY);
            }

        friend bool operator== <>(const VPointT<T>& p1, const VPointT<T>& p2);     // exact equality
        friend bool operator!= <>(const VPointT<T>& p1, const VPointT<T>& p2);     // exact inequality
        friend VPointT<T> operator+ <>(const VPointT<T>& p1, const VPointT<T>& p2); // addition
        friend VPointT<T> operator+ <>(const VPointT<T>& p, const VSizeT<T>& s);     // addition
        friend VPointT<T> operator- <>(const VPointT<T>& p1, const VPointT<T>& p2); // subtraction
        friend VPointT<T> operator- <>(const VPointT<T>& p, const VSizeT<T>& s);     // subtraction
        friend VPointT<T> operator- <>(const VPointT<T>& p);                       // negation
        friend VPointT<T> operator* <>(const VPointT<T>& p, T scale);              // multiplication p*n
        friend VPointT<T> operator* <>(T scale, const VPointT<T>& p);              // multiplication n*p
        friend VPointT<T> operator/ <>(const VPointT<T>& p, T divisor);            // division
        
#ifdef VAULT_QT_SUPPORT
        void setQPoint(const QPoint& p) { mX = p.x(); mY = p.y(); }
        QPoint getQPoint() const { return QPoint(static_cast<int>(mX), static_cast<int>(mY)); }
        void setQPointF(const QPointF& p) { mX = static_cast<T>(p.x()); mY = static_cast<T>(p.y()); }
        QPointF getQPointF() const { return QPointF(mX, mY); }
#endif
        
    private:
    
        T mX;
        T mY;
    };

template<typename T> bool operator==(const VPointT<T>& p1, const VPointT<T>& p2) { return p1.mX == p2.mX && p1.mY == p2.mY; }
template<typename T> bool operator!=(const VPointT<T>& p1, const VPointT<T>& p2) { return p1.mX != p2.mX || p1.mY != p2.mY; }
template<typename T> VPointT<T> operator+(const VPointT<T>& p1, const VPointT<T>& p2) { return VPointT<T>(p1.mX + p2.mX, p1.mY + p2.mY); }
template<typename T> VPointT<T> operator+(const VPointT<T>& p, const VSizeT<T>& s) { return VPointT<T>(p.mX + s.getWidth(), p.mY + s.getHeight()); }
template<typename T> VPointT<T> operator-(const VPointT<T>& p1, const VPointT<T>& p2) { return VPointT<T>(p1.mX - p2.mX, p1.mY - p2.mY); }
template<typename T> VPointT<T> operator-(const VPointT<T>& p, const VSizeT<T>& s) { return VPointT<T>(p.mX - s.getWidth(), p.mY - s.getHeight()); }
template<typename T> VPointT<T> operator-(const VPointT<T>& p) { return VPointT<T>(-p.mX, -p.mY); }
template<typename T> VPointT<T> operator*(const VPointT<T>& p, T scale) { return VPointT<T>(p.mX * scale, p.mY * scale); }
template<typename T> VPointT<T> operator*(T scale, const VPointT<T>& p) { return VPointT<T>(p.mX * scale, p.mY * scale); }
template<typename T> VPointT<T> operator/(const VPointT<T>& p, T divisor) { if (divisor == 0) throw VRangeException("VPointT divide by zero."); return VPointT<T>(p.mX / divisor, p.mY / divisor); }

typedef VPointT<VDouble> VPoint;
typedef VPointT<int> VIPoint;

typedef std::vector<VPoint> VPointVector;
typedef std::vector<VIPoint> VIPointVector;

/**
VPoint3DT defines a point with x, y, and z coordinates. VPoint3D uses VDouble coordinates; VIPoint3D uses int coordinates.
*/
template <typename T> class VPoint3DT;
template <typename T> bool operator== (const VPoint3DT<T>& p1, const VPoint3DT<T>& p2);
template <typename T> bool operator!= (const VPoint3DT<T>& p1, const VPoint3DT<T>& p2);
template <typename T> VPoint3DT<T> operator+ (const VPoint3DT<T>& p1, const VPoint3DT<T>& p2);
template <typename T> VPoint3DT<T> operator- (const VPoint3DT<T>& p1, const VPoint3DT<T>& p2);
template <typename T> VPoint3DT<T> operator- (const VPoint3DT<T>& p);
template <typename T> VPoint3DT<T> operator* (const VPoint3DT<T>& p, T scale);
template <typename T> VPoint3DT<T> operator* (T scale, const VPoint3DT<T>& p);
template <typename T> VPoint3DT<T> operator/ (const VPoint3DT<T>& p, T divisor);

template <typename T>
class VPoint3DT
    {
    public:
    
        VPoint3DT() : mX(0), mY(0), mZ(0) {}
        VPoint3DT(T x, T y, T z) : mX(x), mY(y), mZ(z) {}
        VPoint3DT(VBinaryIOStream& stream) : mX(0), mY(0), mZ(0) { VGeometry::readTripletFromStream(stream, mX, mY, mZ); }
        ~VPoint3DT() {}

        void readFromStream(VBinaryIOStream& stream) { VGeometry::readTripletFromStream(stream, mX, mY, mZ); }
        void writeToStream(VBinaryIOStream& stream) const { VGeometry::writeTripletToStream(stream, mX, mY, mZ); }
        
        T getX() const { return mX; }
        T getY() const { return mY; }
        T getZ() const { return mZ; }
        void setX(T x) { mX = x; }
        void setY(T y) { mY = y; }
        void setZ(T z) { mZ = z; }
        T& rX() { return mX; }
        T& rY() { return mY; }
        T& rZ() { return mZ; }
        void setNull() { setX(0); setY(0); setZ(0); }
        VPoint3DT<T>& operator+=(const VPoint3DT<T>& p) { mX += p.mX; mY += p.mY; mZ += p.mZ; return *this; }
        VPoint3DT<T>& operator-=(const VPoint3DT<T>& p) { mX -= p.mX; mY -= p.mY; mZ -= p.mZ; return *this; }
        VPoint3DT<T>& operator*=(T scale) { mX *= scale; mY *= scale; mZ *= scale; return *this; }
        VPoint3DT<T>& operator/=(T divisor) { if (divisor == 0) throw VRangeException("VPoint3DT divide by zero."); mX /= divisor; mY /= divisor; mZ /= divisor; return *this; }

        // These use "approximate" equality by testing for a difference >= 0.000001.
        static bool equal(const VPoint3DT<T>& p1, const VPoint3DT<T>& p2)
            {
            return VGeometry::equal(p1.mX, p2.mX) && VGeometry::equal(p1.mY, p2.mY) && VGeometry::equal(p1.mZ, p2.mZ);
            }

        static bool notEqual(const VPoint3DT<T>& p1, const VPoint3DT<T>& p2)
            {
            return VGeometry::notEqual(p1.mX, p2.mX) || VGeometry::notEqual(p1.mY, p2.mY) || VGeometry::notEqual(p1.mZ, p2.mZ);
            }

        friend bool operator== <>(const VPoint3DT<T>& p1, const VPoint3DT<T>& p2);     // exact equality
        friend bool operator!= <>(const VPoint3DT<T>& p1, const VPoint3DT<T>& p2);     // exact inequality
        friend VPoint3DT<T> operator+ <>(const VPoint3DT<T>& p1, const VPoint3DT<T>& p2); // addition
        friend VPoint3DT<T> operator- <>(const VPoint3DT<T>& p1, const VPoint3DT<T>& p2); // subtraction
        friend VPoint3DT<T> operator- <>(const VPoint3DT<T>& p);                       // negation
        friend VPoint3DT<T> operator* <>(const VPoint3DT<T>& p, T scale);              // multiplication p*n
        friend VPoint3DT<T> operator* <>(T scale, const VPoint3DT<T>& p);              // multiplication n*p
        friend VPoint3DT<T> operator/ <>(const VPoint3DT<T>& p, T divisor);            // division
        
    private:
    
        T mX;
        T mY;
        T mZ;
    };

template<typename T> bool operator==(const VPoint3DT<T>& p1, const VPoint3DT<T>& p2) { return p1.mX == p2.mX && p1.mY == p2.mY && p1.mZ == p2.mZ; }
template<typename T> bool operator!=(const VPoint3DT<T>& p1, const VPoint3DT<T>& p2) { return p1.mX != p2.mX || p1.mY != p2.mY || p1.mZ != p2.mZ; }
template<typename T> VPoint3DT<T> operator+(const VPoint3DT<T>& p1, const VPoint3DT<T>& p2) { return VPoint3DT<T>(p1.mX + p2.mX, p1.mY + p2.mY, p1.mZ + p2.mZ); }
template<typename T> VPoint3DT<T> operator-(const VPoint3DT<T>& p1, const VPoint3DT<T>& p2) { return VPoint3DT<T>(p1.mX - p2.mX, p1.mY - p2.mY, p1.mZ - p2.mZ); }
template<typename T> VPoint3DT<T> operator-(const VPoint3DT<T>& p) { return VPoint3DT<T>(-p.mX, -p.mY, -p.mZ); }
template<typename T> VPoint3DT<T> operator*(const VPoint3DT<T>& p, T scale) { return VPoint3DT<T>(p.mX * scale, p.mY * scale, p.mZ * scale); }
template<typename T> VPoint3DT<T> operator*(T scale, const VPoint3DT<T>& p) { return VPoint3DT<T>(p.mX * scale, p.mY * scale, p.mZ * scale); }
template<typename T> VPoint3DT<T> operator/(const VPoint3DT<T>& p, T divisor) { if (divisor == 0) throw VRangeException("VPoint3DT divide by zero."); return VPoint3DT<T>(p.mX / divisor, p.mY / divisor, p.mZ / divisor); }

typedef VPoint3DT<VDouble> VPoint3D;
typedef VPoint3DT<int> VIPoint3D;

/**
VLineT defines a line segment with two points. The order matters for equality and math.
Use the same() function to test equality without respect to the order of the two points.
VLine uses VDouble coordinates; VILine uses int coordinates.
*/
template <typename T> class VLineT;
template <typename T> bool operator== (const VLineT<T>& line1, const VLineT<T>& line2); // equality of start and end
template <typename T> bool operator!= (const VLineT<T>& line1, const VLineT<T>& line2); // inequality of start or end

template <typename T>
class VLineT
    {
    public:
    
        VLineT() : mP1(), mP2() {}
        VLineT(VPointT<T> start, VPointT<T> end) : mP1(start), mP2(end) {}
        VLineT(VPointT<T> start, VSizeT<T> vec) : mP1(start), mP2(start + vec) {}
        VLineT(T x1, T y1, T x2, T y2) : mP1(x1, y1), mP2(x2, y2) {}
        VLineT(T x1, T y1, VSizeT<T> vec) : mP1(x1, y1), mP2(VPointT<T>(x1, y1) + vec) {}
        VLineT(VBinaryIOStream& stream) : mP1(), mP2() { mP1.readFromStream(stream); mP2.readFromStream(stream); }
        ~VLineT() {}

#ifdef VAULT_QT_SUPPORT
        VLineT(const QLine& line) : mP1(line.p1()), mP2(line.p2()) {}
        VLineT(const QLineF& line) : mP1(line.p1()), mP2(line.p2()) {}
#endif
            
        void readFromStream(VBinaryIOStream& stream) { mP1.readFromStream(stream); mP2.readFromStream(stream); }
        void writeToStream(VBinaryIOStream& stream) const { mP1.writeToStream(stream); mP2.writeToStream(stream); }
        
        VPointT<T> getP1() const { return mP1; }
        VPointT<T> getP2() const { return mP2; }
        void setP1(VPointT<T> p) { mP1 = p; }
        void setP2(VPointT<T> p) { mP2 = p; }
        void setPoints(VPointT<T> p1, VPointT<T> p2) { mP1 = p1; mP2 = p2; }
        VPointT<T>& rP1() { return mP1; }
        VPointT<T>& rP2() { return mP2; }
        
        VSizeT<T> getSize() const { return VSizeT<T>(this->getDX(), this->getDY()); }
        T getDX() const { return mP2.getX() - mP1.getX(); }
        T getDY() const { return mP2.getY() - mP1.getY(); }
        VDouble getLength() const { return VGeometry::getDistance(this->getDX(), this->getDY()); }
        
        void translate(const VPointT<T>& delta) { mP1 += delta; mP2 += delta; }
        void translate(T deltaX, T deltaY) { this->translate(VPointT<T>(deltaX, deltaY)); }
        VLineT<T> translated(const VPointT<T>& delta) const { VLineT<T> line(*this); line.translate(delta); return line; }
        VLineT<T> translated(T deltaX, T deltaY) const { return this->translated(VPointT<T>(deltaX, deltaY)); }
        
        /**
        Normalizes the line in the following sense: The resulting line's P1 and P2 will point left-to-right (+x),
        and if the x values are the same, it will point top-to-bottom (+y). This can be useful when comparing
        or using two lines when you don't want the "directionality" of the line to matter.
        @return a normalized copy of this line
        */
        VLineT<T> normalized() const
            {
            if (mP1.getX() < mP2.getX())
                {
                return *this;
                }
            else if (mP1.getX() > mP2.getX())
                {
                return VLineT<T>(mP2, mP1);
                }
            else if (mP1.getY() < mP2.getY())
                {
                return *this;
                }

            return VLineT<T>(mP2, mP1);
            }
        
        VLineT<T> reversed() const { return VLineT<T>(mP2, mP1); }
        
        /**
        Returns a VLine starting at P1, towards P2 but with length 1.0.
        @return a unit vector of this line
        */
        VLineT<VDouble> getUnitVector() const
            {
            VDouble length = this->getLength();
            if (length == 0.0)
                return VLineT<T>();
            
            return VLineT<T>(mP1, (this->getSize() / length));
            }

        // These use "approximate" equality by testing for a difference >= 0.000001.
        static bool equal(const VLineT<T>& line1, const VLineT<T>& line2)
            {
            return VPointT<T>::equal(line1.mP1, line2.mP1) && VPointT<T>::equal(line1.mP2, line2.mP2);
            }

        static bool notEqual(const VLineT<T>& line1, const VLineT<T>& line2)
            {
            return VPointT<T>::notEqual(line1.mP1, line2.mP1) || VPointT<T>::notEqual(line1.mP2, line2.mP2);
            }

        /**
        Returns true if the two lines have the same endpoints (within 0.000001), regardless of their order.
        @param  line1   a line
        @param  line2   a line
        @return true if line1 and line2 have the same endpoints
        */
        static bool same(const VLineT<T>& line1, const VLineT<T>& line2)
            {
            return VLineT<T>::equal(line1.normalized(), line2.normalized());
            }

        friend bool operator== <>(const VLineT<T>& line1, const VLineT<T>& line2);     // exact equality
        friend bool operator!= <>(const VLineT<T>& line1, const VLineT<T>& line2);     // exact inequality
        
#ifdef VAULT_QT_SUPPORT
        void setQLine(const QLine& line) { mP1 = line.p1(); mP2 = line.p2(); }
        QLine getQLine() const { return QLine(mP1.getQPoint(), mP2.getQPoint()); }
        void setQLineF(const QLineF& line) { mP1.setQPointF(line.p1()); mP2.setQPointF(line.p2()); }
        QLineF getQLineF() const { return QLineF(mP1, mP2); }
#endif

        /**
        Returns the point on this line (AB) that is the nearest to the supplied point C.
        You can specify whether this line is treated as a line segment (result within
        the segment) or not (result may be outside the segment).
        Note the input and result points are VPoint, not VPointT<T>, because even if
        this line is integer-based, the result may not be.
        @param  C                   a point to test
        @param  restrictToSegment   true if the result must lie between endpoints
        @return the point along this line (or within segment) that is nearest to C
        */
        VPoint getNearestPoint(const VPoint& C, bool restrictToSegment) const
            {
            // Name shortcuts for readability.
            const VDouble cx = C.getX();
            const VDouble cy = C.getY();
            const VDouble ax = mP1.getX();
            const VDouble ay = mP1.getY();
            const VDouble bx = mP2.getX();
            const VDouble by = mP2.getY();
            
            const VDouble ratioABNumerator = (cx-ax)*(bx-ax) + (cy-ay)*(by-ay);
            const VDouble ratioABDenomenator = (bx-ax)*(bx-ax) + (by-ay)*(by-ay);
            const VDouble ratioAB = ratioABNumerator / ratioABDenomenator; // how far along the segment A->B is nearest point (0.0 = @A, 1.0 = @B)

            // P is the result. We may need to restrict it to the segment ( next.
            VPoint P(ax + (ratioAB*(bx-ax)), ay + (ratioAB*(by-ay)));

// You can uncomment these if needed for debugging.
//            const VDouble s = ((ay-cy)*(bx-ax)-(ax-cx)*(by-ay) ) / ratioABDenomenator;
//            const VDouble distanceToLine = V_FABS(s) * sqrt(ratioABDenomenator);
//            VDouble distanceToSegment = 0.0;
            
            if (((ratioAB >= 0.0) && (ratioAB <= 1.0)) || !restrictToSegment) // nearest point is inside segment, or we don't care that it's outside
                {
//                distanceToSegment = distanceToLine;
                }
            else
                {
                // Select the nearest endpoint of the segment.
                // Avoid calling sqrt until needed.
                const VDouble P1_distance_squared = (cx-ax)*(cx-ax) + (cy-ay)*(cy-ay);
                const VDouble P2_distance_squared = (cx-bx)*(cx-bx) + (cy-by)*(cy-by);
                if (P1_distance_squared < P2_distance_squared)
                    {
                    P = mP1;
//                    distanceToSegment = sqrt(P1_distance_squared);
                    }
                else
                    {
                    P = mP2;
//                    distanceToSegment = sqrt(P2_distance_squared);
                    }
                }

            return P;
            }

        VDouble getDistanceToPoint(const VPoint& p, bool restrictToSegment) const
            {
            VPoint nearestPointOnLine = this->getNearestPoint(p, restrictToSegment);
            return VPoint::getDistance(p, nearestPointOnLine);
            }
        
    private:
    
        VPointT<T> mP1;
        VPointT<T> mP2;
    };

template<typename T> bool operator==(const VLineT<T>& line1, const VLineT<T>& line2) { return line1.mP1 == line2.mP1 && line1.mP2 == line2.mP2; }
template<typename T> bool operator!=(const VLineT<T>& line1, const VLineT<T>& line2) { return line1.mP1 != line2.mP1 || line1.mP2 != line2.mP2; }

typedef VLineT<VDouble> VLine;
typedef VLineT<int> VILine;

template <typename T> class VPolygonT; // forward declaration for use by VRectT

/**
VRectT defines a rectangle. VRect uses VDouble coordinates; VIRect uses int coordinates.
*/
template <typename T> class VRectT;
template <typename T> bool operator== (const VRectT<T>& p1, const VRectT<T>& p2);
template <typename T> bool operator!= (const VRectT<T>& p1, const VRectT<T>& p2);

template <typename T>
class VRectT
    {
    public:
    
        VRectT() : mLeftTop(), mSize() {}
        VRectT(const VPointT<T>& leftTop, const VSizeT<T>& size) : mLeftTop(leftTop), mSize(size) {}
        VRectT(const VPointT<T>& leftTop, const VPointT<T>& rightBottom) : mLeftTop(leftTop), mSize(rightBottom.getX() - leftTop.getX(), rightBottom.getY() - leftTop.getY()) {}
        VRectT(VBinaryIOStream& stream) : mLeftTop(stream), mSize(stream) {}
        VRectT(T left, T top, T width, T height) : mLeftTop(left, top), mSize(width, height) {}
        ~VRectT() {}

#ifdef VAULT_QT_SUPPORT
        VRectT(const QRect& r) : mLeftTop(r.topLeft()), mSize(r.size()) {}
        VRectT(const QRectF& r) : mLeftTop(static_cast<VPointT<T> >(r.topLeft())), mSize(static_cast<VSizeT<T> >(r.size())) {}
#endif
            
        void readFromStream(VBinaryIOStream& stream) { mLeftTop.readFromStream(stream); mSize.readFromStream(stream); }
        void writeToStream(VBinaryIOStream& stream) const { mLeftTop.writeToStream(stream); mSize.writeToStream(stream); }
        
        VPointT<T> getLeftTop() const { return mLeftTop; }
        VPointT<T> getRightBottom() const { return VPointT<T>(mLeftTop + mSize); }
        VPointT<T> getRightTop() const { return VPointT<T>(mLeftTop + VSizeT<T>(mSize.getWidth(), 0)); }
        VPointT<T> getLeftBottom() const { return VPointT<T>(mLeftTop + VSizeT<T>(0, mSize.getHeight())); }
        VSizeT<T> getSize() const { return mSize; }
        T getLeft() const { return mLeftTop.getX(); }
        T getTop() const { return mLeftTop.getY(); }
        T getRight() const { return mLeftTop.getX() + mSize.getWidth(); }
        T getBottom() const { return mLeftTop.getY() + mSize.getHeight(); }
        T getWidth() const { return mSize.getWidth(); }
        T getHeight() const { return mSize.getHeight(); }
        VPointT<T> getCenter() const { return VPointT<T>(mLeftTop.getX() + (mSize.getWidth() / 2), mLeftTop.getY() + (mSize.getHeight() / 2)); }
        /** The "edge" functions return lines that are "normalized", preferring to point +x, then +y. */
        VLineT<T> getTopSide() const { return VLineT<T>(this->getLeftTop(), this->getRightTop()).normalized(); }
        VLineT<T> getRightSide() const { return VLineT<T>(this->getRightTop(), this->getRightBottom()).normalized(); }
        VLineT<T> getBottomSide() const { return VLineT<T>(this->getRightBottom(), this->getLeftBottom()).normalized(); }
        VLineT<T> getLeftSide() const { return VLineT<T>(this->getLeftBottom(), this->getLeftTop()).normalized(); }

        VRectT<T> normalized() const
            {
            VPointT<T> p(mLeftTop);
            VSizeT<T> s(mSize);
            
            T width = s.getWidth();
            if (width < 0)
                {
                s.setWidth(-width);
                p.rX() += width; // width is negative at this point
                }
            
            T height = s.getHeight();
            if (height < 0)
                {
                s.setHeight(-height);
                p.rY() += height; // height is negative at this point
                }
            
            return VRectT<T>(p, s);
            }

        void moveTo(const VPointT<T>& leftTop) { mLeftTop = leftTop; }
        void translate(T dx, T dy) { mLeftTop += VPointT<T>(dx, dy); }
        void translate(const VPointT<T>& offset) { mLeftTop += offset; }
        void setSize(const VSizeT<T>& size) { mSize = size; }
        void setWidth(T width) { mSize.setWidth(width); }
        void setHeight(T height) { mSize.setHeight(height); }
        void setBounds(T left, T top, T right, T bottom) { mLeftTop.setX(left); mLeftTop.setY(top); mSize.setWidth(right - left); mSize.setHeight(bottom - top); }
        void setLeft(T left) { mLeftTop.setX(left); }
        void setTop(T top) { mLeftTop.setY(top); }
        void setRight(T right) { mSize.setWidth(right - this->getLeft()); }
        void setBottom(T bottom) { mSize.setHeight(bottom - this->getTop()); }

        bool contains(const VPointT<T>& p) const
            {
            VPointT<T> rightBottom = this->getRightBottom();

            return p.getX() >= mLeftTop.getX() && p.getX() < rightBottom.getX() &&
                    p.getY() >= mLeftTop.getY() && p.getY() < rightBottom.getY();
            }

        void expandTo(const VPointT<T>& p)
            {
            if (! this->contains(p))
                {
                T leftDX = mLeftTop.getX() - p.getX();
                if (leftDX > 0)
                    {
                    mLeftTop.rX() -= leftDX;
                    mSize.rWidth() += leftDX;
                    }
                else
                    {
                    T rightDX = p.getX() - this->getRight();
                    if (rightDX > 0)
                        {
                        mSize.rWidth() += rightDX;
                        }
                    }

                T topDY = mLeftTop.getY() - p.getY();
                if (topDY > 0)
                    {
                    mLeftTop.rY() -= topDY;
                    mSize.rHeight() += topDY;
                    }
                else
                    {
                    T bottomDY = p.getY() - this->getBottom();
                    if (bottomDY > 0)
                        {
                        mSize.rHeight() += bottomDY;
                        }
                    }

                }
            }

        VRectT<T> united(const VRectT<T>& r) const
            {
            VRectT<T> result(mLeftTop, mSize);
            result.expandTo(r.getLeftTop());
            result.expandTo(r.getRightBottom());
            
            return result;
            }

        VRectT<T> intersected(const VRectT<T>& r) const
            {
            T left = V_MAX(this->getLeft(), r.getLeft());
            T top = V_MAX(this->getTop(), r.getTop());
            T right = V_MIN(this->getRight(), r.getRight());
            T bottom = V_MIN(this->getBottom(), r.getBottom());
            T width = right - left;
            T height = bottom - top;
            
            if ((width >= 0) && (height >= 0))
                return VRectT<T>(left, top, width, height);
            
            return VRectT<T>();
            }

        /**
        Returns the corner of the rectangle nearest to the supplied point.
        @param  testPoint   a point to test proximity
        @return the corner nearest to the test point
        */
        VPointT<T> getNearestVertex(const VPointT<T>& testPoint) const
            {
            VPolygonT<T> polygon(*this);
            int nearestVertex = polygon.getNearestVertex(testPoint);
            return polygon.getPoint(nearestVertex);
            }

        /**
        Returns the side of the rectangle that is nearest to the supplied point.
        If the test point is in a "dead zone" where the nearest point on a side is a vertex,
        there is currently no guarantee which of the two candidate sides will be returned.
        @param  testPoint   a point to test proximity
        @return a line defining a side of the rectangle the point is nearest to
        @see "dead zone" comments at the end of this file
        */
        VLineT<T> getNearestSide(const VPointT<T>& testPoint) const
            {
            VPointT<T> unused;
            return this->getNearestSide(testPoint, unused);
            }

        /**
        Returns the side of the rectangle that is nearest to the supplied point.
        If the test point is in a "dead zone" where the nearest point on a side is a vertex,
        there is currently no guarantee which of the two candidate sides will be returned,
        and the returned nearestPoint will a vertex (shared by the two possible sides).
        @param  testPoint   a point to test proximity
        @param  nearestPoint returned value, the point (on the line segment) that is nearest to
                                the test point
        @return a line defining a side of the rectangle the point is nearest to
        @see "dead zone" comments at the end of this file
        */
        VLineT<T> getNearestSide(const VPointT<T>& testPoint, VPoint& nearestPoint) const
            {
            VPolygonT<T> polygon(*this);
            int nearestSide = polygon.getNearestSide(testPoint, nearestPoint);
            return polygon.getSide(nearestSide);
            }

        // These use "approximate" equality by testing for a difference >= 0.000001.
        static bool equal(const VRectT<T>& r1, const VRectT<T>& r2)
            {
            return VPointT<T>::equal(r1.mLeftTop, r2.mLeftTop) && VSizeT<T>::equal(r1.mSize, r2.mSize);
            }

        static bool notEqual(const VRectT<T>& r1, const VRectT<T>& r2)
            {
            return VPointT<T>::notEqual(r1.mLeftTop, r2.mLeftTop) || VSizeT<T>::notEqual(r1.mSize, r2.mSize);
            }

        friend bool operator== <>(const VRectT<T>& r1, const VRectT<T>& r2);     // exact equality
        friend bool operator!= <>(const VRectT<T>& r1, const VRectT<T>& r2);     // exact inequality
    
#ifdef VAULT_QT_SUPPORT
        void setQRect(const QRect& r) { mLeftTop = r.topLeft(); mSize = r.size(); }
        QRect getQRect() const { return QRect(static_cast<int>(mLeftTop), static_cast<int>(mSize)); }
        void setQRectF(const QRectF& r) { mLeftTop = static_cast<T>(r.topLeft()); mSize = static_cast<T>(r.size()); }
        QRectF getQRectF() const { return QRectF(mLeftTop, mSize); }
#endif
        
    private:
    
        VPointT<T> mLeftTop;
        VSizeT<T> mSize;
    };

template<typename T> bool operator==(const VRectT<T>& r1, const VRectT<T>& r2) { return r1.mLeftTop == r2.mLeftTop && r1.mSize == r2.mSize; }
template<typename T> bool operator!=(const VRectT<T>& r1, const VRectT<T>& r2) { return r1.mLeftTop != r2.mLeftTop || r1.mSize != r2.mSize; }

typedef VRectT<VDouble> VRect;
typedef VRectT<int> VIRect;

/**
VPolygonT defines a polygon or series of points. VPolygon uses VDouble coordinates; VIPolygon uses int coordinates.
*/
template <typename T> class VPolygonT;
template <typename T> bool operator== (const VPolygonT<T>& p1, const VPolygonT<T>& p2);
template <typename T> bool operator!= (const VPolygonT<T>& p1, const VPolygonT<T>& p2);

template <typename T>
class VPolygonT
    {
    public:
    
        VPolygonT() : mPoints() {}
        VPolygonT(const std::vector<VPointT<T> >& points) : mPoints(points) {}
        VPolygonT(VBinaryIOStream& stream) : mPoints() { this->readFromStream(stream); }
        VPolygonT(const VRectT<T>& rect) :
            mPoints()
            {
            this->add(rect.getLeftTop());
            this->add(rect.getRightTop());
            this->add(rect.getRightBottom());
            this->add(rect.getLeftBottom());
            }

        ~VPolygonT() {}

#ifdef VAULT_QT_SUPPORT
        VPolygonT(const QPolygon& p)
            {
            for (QPolygon::const_iterator i = p.begin(); i != p.end(); ++i)
                this->add(VPointT<T>(static_cast<T>(i->x()), static_cast<T>(i->y())));
            }

        VPolygonT(const QPolygonF& p)
            {
            for (QPolygonF::const_iterator i = p.begin(); i != p.end(); ++i)
                this->add(VPointT<T>(i->x(), i->y()));
            }
#endif
            
        void readFromStream(VBinaryIOStream& stream)
            {
            this->eraseAll();
            int numPoints = static_cast<int>(stream.readS32());
            for (int i = 0; i < numPoints; ++i)
                {
                VPointT<T> point;
                point.readFromStream(stream);
                this->add(point);
                }
            }

        void writeToStream(VBinaryIOStream& stream) const
            {
            int numPoints = this->getNumPoints();
            stream.writeInt32(numPoints);
            for (int i = 0; i < numPoints; ++i)
                {
                mPoints[i].writeToStream(stream);
                }
            }
        
        void add(VPointT<T> p) { mPoints.push_back(p); }
        void insert(int beforeIndex, VPointT<T> p) { mPoints.insert(mPoints.begin() + beforeIndex, p); }
        int getNumPoints() const { return static_cast<int>(mPoints.size()); }
        size_t size() const { return mPoints.size(); }
        const std::vector<VPointT<T> >& getPoints() const { return mPoints; }
        VPointT<T> getPoint(int index) const { if (index >= this->getNumPoints()) { throw VRangeException(VSTRING_FORMAT("getPoint: Invalid index %d into %d-point polygon.", index, this->getNumPoints())); } return mPoints[index]; }
        void setPoint(int index, VPointT<T> p) { if (index >= this->getNumPoints()) { throw VRangeException(VSTRING_FORMAT("setPoint: Invalid index %d into %d-point polygon.", index, this->getNumPoints())); } mPoints[index] = p; }
        VPointT<T>& operator[](int index) { return mPoints[index]; } // Note that operator[] by definition does no range checking.
        VPointT<T> operator[](int index) const { return mPoints[index]; } // Note that operator[] by definition does no range checking.

        VRectT<T> getBounds() const
            {
            VRectT<T> bounds; // defaults to a zero rect
            int numPoints = this->getNumPoints();
            
            if (numPoints > 0)
                {
                bounds.moveTo(mPoints[0]);
                
                for (int i = 1; i < numPoints; ++i)
                    bounds.expandTo(mPoints[i]);

                }
            
            return bounds;
            }

        void remove(int index) { mPoints.erase(mPoints.begin() + index); }
        void eraseAll() { mPoints.erase(mPoints.begin(), mPoints.end()); }
        void translate(VPointT<T> offset)
            {
            int numPoints = static_cast<int>(mPoints.size());
            for (int i = 0; i < numPoints; ++i)
                mPoints[i] += offset;
            }
        
        VLineT<T> getSide(int index) const
            {
            int maxValidIndex = this->getNumPoints() - 1;
            if (index > maxValidIndex)
                {
                throw VRangeException(VSTRING_FORMAT("getSide: Invalid index %d into %d-point polygon.", index, this->getNumPoints()));
                }

            int index2 = (index == maxValidIndex) ? 0 : index+1;
            return VLineT<T>(mPoints[index], mPoints[index2]);
            }
        
        /**
        Returns the index of the vertex of the polygon nearest to the supplied point.
        @param  testPoint   a point to test proximity
        @return the vertex nearest to the test point
        */
        int getNearestVertex(const VPointT<T>& testPoint) const
            {
            int nearestVertexIndex = -1;
            VDouble nearestVertexDistance = -1.0;
            int numPoints = static_cast<int>(mPoints.size());
            for (int i = 0; i < numPoints; ++i)
                {
                VDouble distanceToVertex = VPoint::getDistance(testPoint, mPoints[i]);
                if ((nearestVertexIndex < 0) || (distanceToVertex < nearestVertexDistance))
                    {
                    nearestVertexIndex = i;
                    nearestVertexDistance = distanceToVertex;
                    }
                }
            
            return nearestVertexIndex;
            }

        /**
        Returns the index of the side of the polygon that is nearest to the supplied point.
        If the test point is in a "dead zone" where the nearest point on a side is a vertex,
        there is currently no guarantee which of the two candidate sides will be returned.
        @param  testPoint   a point to test proximity
        @return the index of the side of the polygon the point is nearest to
        @see "dead zone" comments at the end of this file
        */
        int getNearestSide(const VPointT<T>& testPoint) const
            {
            VPointT<T> unused;
            return this->getNearestSide(testPoint, unused);
            }

        /**
        Returns the index of the side of the polygon that is nearest to the supplied point.
        If the test point is in a "dead zone" where the nearest point on a side is a vertex,
        there is currently no guarantee which of the two candidate sides will be returned,
        and the returned nearestPoint will a vertex (shared by the two possible sides).
        @param  testPoint   a point to test proximity
        @param  nearestPoint returned value, the point (on the side) that is nearest to
                                the test point
        @return the index of the side of the polygon the point is nearest to
        @see "dead zone" comments at the end of this file
        */
        int getNearestSide(const VPointT<T>& testPoint, VPoint& nearestPoint) const
            {
            int nearestSideIndex = -1;
            VDouble nearestSideDistance = -1.0;
            VPoint nearestSidePoint;
            int numSides = static_cast<int>(mPoints.size());
            for (int i = 0; i < numSides; ++i)
                {
                VLine side = this->getSide(i);
                VPoint p = side.getNearestPoint(testPoint, true);
                VDouble distanceToSide = VPoint::getDistance(testPoint, p);
                if ((nearestSideIndex < 0) || (distanceToSide < nearestSideDistance))
                    {
                    nearestSideIndex = i;
                    nearestSidePoint = p;
                    nearestSideDistance = distanceToSide;
                    }
                }
            
            nearestPoint = nearestSidePoint;
            return nearestSideIndex;
            }

        // These use "approximate" equality by testing for a coordinate difference >= 0.000001.
        static bool equal(const VPolygonT<T>& p1, const VPolygonT<T>& p2)
            {
            if (p1.mPoints.size() != p2.mPoints.size())
                return false;
            
            int numPoints = static_cast<int>(p1.mPoints.size());
            for (int i = 0; i < numPoints; ++i)
                {
                if (VPointT<T>::notEqual(p1.mPoints[i], p2.mPoints[i]))
                    return false;
                }
            
            return true;
            }

        static bool notEqual(const VPolygonT<T>& p1, const VPolygonT<T>& p2)
            {
            if (p1.mPoints.size() == p2.mPoints.size())
                return false;
            
            int numPoints = static_cast<int>(p1.mPoints.size());
            for (int i = 0; i < numPoints; ++i)
                {
                if (VPointT<T>::equal(p1.mPoints[i], p2.mPoints[i]))
                    return false;
                }
            
            return true;
            }

        friend bool operator== <>(const VPolygonT<T>& p1, const VPolygonT<T>& p2);     // exact equality
        friend bool operator!= <>(const VPolygonT<T>& p1, const VPolygonT<T>& p2);     // exact inequality

#ifdef VAULT_QT_SUPPORT
        void setQPolygon(const QPolygon& p)
            {
            this->eraseAll();
            for (QPolygon::const_iterator i = p.begin(); i != p.end(); ++i)
                this->add(VPointT<T>(static_cast<T>(i->x()), static_cast<T>(i->y())));
            }

        QPolygon getQPolygon() const
            {
            QPolygon p;
            int numPoints = static_cast<int>(mPoints.size());
            for (int i = 0; i < numPoints; ++i)
                p.push_back(mPoints[i].getQPoint());
            return p;
            }

        void setQPolygonF(const QPolygonF& p)
            {
            this->eraseAll();
            for (QPolygonF::const_iterator i = p.begin(); i != p.end(); ++i)
                this->add(VPointT<T>(i->x(), i->y()));
            }

        QPolygonF getQPolygonF() const
            {
            QPolygonF p;
            int numPoints = static_cast<int>(mPoints.size());
            for (int i = 0; i < numPoints; ++i)
                p.push_back(mPoints[i].getQPointF());
            return p;
            }
#endif
        
    private:
    
        std::vector<VPointT<T> > mPoints;
    };

template<typename T> bool operator==(const VPolygonT<T>& p1, const VPolygonT<T>& p2) { return p1.mPoints == p2.mPoints; }
template<typename T> bool operator!=(const VPolygonT<T>& p1, const VPolygonT<T>& p2) { return p1.mPoints != p2.mPoints; }

typedef VPolygonT<VDouble> VPolygon;
typedef VPolygonT<int> VIPolygon;

/*
Comments about "dead zones":

There are several functions in VRectT and VPolygonT that locate the "nearest" side of
the rectangle or polygon to a particular point in 2D space. There are some cases where
the point is in a "dead zone", such that the answer is ambiguous. I have a diagram
showing this at the following URL:
<http://www.bombaydigital.com/software/vault/images/deadzone.png>

The basic problem is that if the nearest point on the edge of the rectangle or polygon
is a vertex (a corner), then it is not clear which of the two sides attached at the vertex
should be considered "nearest" to the point. The current implementation makes no guarantee
about which of the two sides will be chosen in this case. If you ask which vertex is nearest,
the answer is unambiguous. It's the "nearest side" question that has an ambigous answer.
Fortunately, the use cases I can think of don't care which side is returned because they
are really trying to get the nearest point on that side, which is the same for either side.
*/

#endif /* vgeometry_h */
