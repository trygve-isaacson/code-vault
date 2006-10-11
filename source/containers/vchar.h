/*
Copyright c1997-2006 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 2.5
http://www.bombaydigital.com/
*/

#ifndef vchar_h
#define vchar_h

/** @file */

#include "vtypes.h"

/**
    @ingroup vstring
*/

/**
VChar is a simple wrapper for C char values.

It provides easier and safer
ways of setting, converting, comparing, and modifying characters.

This class does not attempt to do lexical comparisons or deal with
Unicode or character set issues. It is simply a lightweight object
replacement for using char values directly in source code.

@see    VString
*/

class VChar
    {
    public:
    
        /**
        Constructs a character with value zero.
        */
        VChar();
        /**
        Constructs a character from a char.
        @param c the char
        */
        VChar(char c);
        /**
        Constructs a character from an int for conversion purposes.
        The integer value is cast to char.
        @param i the int
        */
        VChar(int i);

        /**
        Assigns the character from a C char.
        @param    c    the char
        */
        VChar& operator=(char c);
        /**
        Assigns the character from an int specifying the 8-bit char value.
        @param    i    the integer value of the char
        */
        VChar& operator=(int i);
        
        /**
        Returns true if the character is a lower case character.
        @return true if the character is a lower case character
        */
        bool    isLowerCase() const;
        /**
        Returns a lower-case version of the character.
        @return    the lower-case value
        */
        VChar    lowerCase() const;
        /**
        Converts the character to lower-case.
        */
        void    toLowerCase();
        /**
        Returns true if the character is an upper case character.
        @return true if the character is an upper case character
        */
        bool    isUpperCase() const;
        /**
        Returns an upper-case version of the character.
        @return    the upper-case value
        */
        VChar    upperCase() const;
        /**
        Converts the character to upper-case.
        */
        void    toUpperCase();
        /**
        Returns the char value of the character.
        @return    the char value
        */
        char    charValue() const;
        /**
        Returns the int value of the character.
        @return the int value
        */
        int        intValue() const;
        /**
        Sets the character from a char, as in the char constructor.
        @param c the char
        */
        void    set(char c);
        /**
        Sets the character from an int, as in the int constructor.
        @param i the int
        */
        void    set(int i);
        /**
        Coerces the character to a char.
        @return    the char value
        */
        operator char() const;
        
        // These utilities only make sense for simple ASCII parsing purposes;
        // they only look at values in the base ASCII range below 128.
        bool isAlpha() const;            ///< Returns true if the character is A-Z or a-z.
        bool isNumeric() const;            ///< Returns true if the character is a digit.
        bool isAlphaNumeric() const;    ///< Returns true if the character is alpha or numeric.
        bool isWhitespace() const;        ///< Returns true if the character is non-printing.

        friend inline bool operator==(const VChar& c1, const VChar& c2);
        friend inline bool operator==(const VChar& c1, char c2);
        friend inline bool operator==(char c1, const VChar& c2);
    
        friend inline bool operator!=(const VChar& c1, const VChar& c2);
        friend inline bool operator!=(const VChar& c1, char c2);
        friend inline bool operator!=(char c1, const VChar& c2);
    
        friend inline bool operator>=(const VChar& c1, const VChar& c2);
        friend inline bool operator>=(const VChar& c1, char c2);
        friend inline bool operator>=(char c1, const VChar& c2);
    
        friend inline bool operator<=(const VChar& c1, const VChar& c2);
        friend inline bool operator<=(const VChar& c1, char c2);
        friend inline bool operator<=(char c1, const VChar& c2);
    
        friend inline bool operator>(const VChar& c1, const VChar& c2);
        friend inline bool operator>(const VChar& c1, char c2);
        friend inline bool operator>(char c1, const VChar& c2);
    
        friend inline bool operator<(const VChar& c1, const VChar& c2);
        friend inline bool operator<(const VChar& c1, char c2);
        friend inline bool operator<(char c1, const VChar& c2);
        
    private:
    
        char    mValue;    ///< The character value.
    };

inline bool operator==(const VChar& c1, const VChar& c2) { return c1.mValue == c2.mValue; }    ///< Compares c1 and c2. @param    c1 a VChar @param c2 a VChar @return true if c1 == c2
inline bool operator==(const VChar& c1, char c2) { return c1.mValue == c2; }                ///< Compares c1 and c2. @param    c1 a VChar @param c2 a char @return true if c1 == c2
inline bool operator==(char c1, const VChar& c2) { return c1 == c2.mValue; }                ///< Compares c1 and c2. @param    c1 a char @param c2 a VChar @return true if c1 == c2

inline bool operator!=(const VChar& c1, const VChar& c2) { return c1.mValue != c2.mValue; }    ///< Compares c1 and c2. @param    c1 a VChar @param c2 a VChar @return true if c1 != c2
inline bool operator!=(const VChar& c1, char c2) { return c1.mValue != c2; }                ///< Compares c1 and c2. @param    c1 a VChar @param c2 a char @return true if c1 != c2
inline bool operator!=(char c1, const VChar& c2) { return c1 != c2.mValue; }                ///< Compares c1 and c2. @param    c1 a char @param c2 a VChar @return true if c1 != c2

inline bool operator>=(const VChar& c1, const VChar& c2) { return c1.mValue >= c2.mValue; }    ///< Compares c1 and c2. @param    c1 a VChar @param c2 a VChar @return true if c1 >= c2
inline bool operator>=(const VChar& c1, char c2) { return c1.mValue >= c2; }                ///< Compares c1 and c2. @param    c1 a VChar @param c2 a char @return true if c1 >= c2
inline bool operator>=(char c1, const VChar& c2) { return c1 >= c2.mValue; }                ///< Compares c1 and c2. @param    c1 a char @param c2 a VChar @return true if c1 >= c2

inline bool operator<=(const VChar& c1, const VChar& c2) { return c1.mValue <= c2.mValue; }    ///< Compares c1 and c2. @param    c1 a VChar @param c2 a VChar @return true if c1 <= c2
inline bool operator<=(const VChar& c1, char c2) { return c1.mValue <= c2; }                ///< Compares c1 and c2. @param    c1 a VChar @param c2 a char @return true if c1 <= c2
inline bool operator<=(char c1, const VChar& c2) { return c1 <= c2.mValue; }                ///< Compares c1 and c2. @param    c1 a char @param c2 a VChar @return true if c1 <= c2

inline bool operator>(const VChar& c1, const VChar& c2) { return c1.mValue > c2.mValue; }    ///< Compares c1 and c2. @param    c1 a VChar @param c2 a VChar @return true if c1 > c2
inline bool operator>(const VChar& c1, char c2) { return c1.mValue > c2; }                    ///< Compares c1 and c2. @param    c1 a VChar @param c2 a char @return true if c1 > c2
inline bool operator>(char c1, const VChar& c2) { return c1 > c2.mValue; }                    ///< Compares c1 and c2. @param    c1 a char @param c2 a VChar @return true if c1 > c2

inline bool operator<(const VChar& c1, const VChar& c2) { return c1.mValue < c2.mValue; }    ///< Compares c1 and c2. @param    c1 a VChar @param c2 a VChar @return true if c1 < c2
inline bool operator<(const VChar& c1, char c2) { return c1.mValue < c2; }                    ///< Compares c1 and c2. @param    c1 a VChar @param c2 a char @return true if c1 < c2
inline bool operator<(char c1, const VChar& c2) { return c1 < c2.mValue; }                    ///< Compares c1 and c2. @param    c1 a char @param c2 a VChar @return true if c1 < c2

#endif /* vchar_h */
