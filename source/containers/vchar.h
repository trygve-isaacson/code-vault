/*
Copyright c1997-2011 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 3.3
http://www.bombaydigital.com/
*/

#ifndef vchar_h
#define vchar_h

/** @file */

#include "vtypes.h"

/**
    @ingroup vstring
*/

/** Simple C char value for a null terminator of a string. Prefer VChar::NULL_CHAR() unless referencing plain char value. */
#define VCHAR_NULL_TERMINATOR '\0'

/**
VChar is a simple wrapper for C char values.

It provides easier and safer
ways of setting, converting, comparing, and modifying characters.

This class does not attempt to do lexical comparisons or deal with
Unicode or character set issues. It is simply a lightweight object
replacement for using char values directly in source code.

@see    VString
*/

class VChar {
    public:

        /**
        Returns a reference to the read-only null character value.
        */
        static const VChar& NULL_CHAR();

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
        bool isLowerCase() const;
        /**
        Returns a lower-case version of the character.
        @return    the lower-case value
        */
        VChar lowerCase() const;
        /**
        Converts the character to lower-case.
        */
        void toLowerCase();
        /**
        Returns true if the character is an upper case character.
        @return true if the character is an upper case character
        */
        bool isUpperCase() const;
        /**
        Returns an upper-case version of the character.
        @return    the upper-case value
        */
        VChar upperCase() const;
        /**
        Converts the character to upper-case.
        */
        void toUpperCase();
        /**
        Returns the char value of the character.
        @return    the char value
        */
        char charValue() const;
        /**
        Returns the int value of the character.
        @return the int value
        */
        int intValue() const;
        /**
        Sets the character from a char, as in the char constructor.
        @param c the char
        */
        void set(char c);
        /**
        Sets the character from an int, as in the int constructor.
        @param i the int
        */
        void set(int i);
        /**
        Coerces the character to a char.
        @return    the char value
        */
        operator char() const;

        // These utilities only make sense for simple ASCII parsing purposes;
        // they only look at values in the base ASCII range below 128.
        bool isAlpha() const;           ///< Returns true if the character is A-Z or a-z.
        bool isNumeric() const;         ///< Returns true if the character is a digit.
        bool isAlphaNumeric() const;    ///< Returns true if the character is alpha or numeric.
        bool isWhitespace() const;      ///< Returns true if the character is non-printing.
        bool isHexadecimal() const;     ///< Returns true if the character is 0-9, A-F, or a-f.

        static bool equalsIgnoreCase(const VChar& c1, const VChar& c2); ///< Returns true if two characters are equal regardless of case.
        static bool equalsIgnoreCase(const VChar& c1, char c2);         ///< Returns true if two characters are equal regardless of case.
        static bool equalsIgnoreCase(char c1, const VChar& c2);         ///< Returns true if two characters are equal regardless of case.
        static bool equalsIgnoreCase(char c1, char c2);                 ///< Returns true if two characters are equal regardless of case.

        friend inline bool operator==(const VChar& lhs, const VChar& rhs);
#ifndef VPLATFORM_DISALLOW_VCHAR_OPERATOR_EQUALS_CHAR
        friend inline bool operator==(const VChar& lhs, char rhs);
        friend inline bool operator==(char lhs, const VChar& rhs);
#endif

        friend inline bool operator!=(const VChar& lhs, const VChar& rhs);
#ifndef VPLATFORM_DISALLOW_VCHAR_OPERATOR_EQUALS_CHAR
        friend inline bool operator!=(const VChar& lhs, char rhs);
        friend inline bool operator!=(char lhs, const VChar& rhs);
#endif

        friend inline bool operator<(const VChar& lhs, const VChar& rhs);
        friend inline bool operator<(const VChar& lhs, char rhs);
        friend inline bool operator<(char lhs, const VChar& rhs);

        friend inline bool operator<=(const VChar& lhs, const VChar& rhs);
        friend inline bool operator<=(const VChar& lhs, char rhs);
        friend inline bool operator<=(char lhs, const VChar& rhs);

        friend inline bool operator>=(const VChar& lhs, const VChar& rhs);
        friend inline bool operator>=(const VChar& lhs, char rhs);
        friend inline bool operator>=(char lhs, const VChar& rhs);

        friend inline bool operator>(const VChar& lhs, const VChar& rhs);
        friend inline bool operator>(const VChar& lhs, char rhs);
        friend inline bool operator>(char lhs, const VChar& rhs);

    private:

        char mValue;    ///< The character value.
};

inline bool operator==(const VChar& lhs, const VChar& rhs) { return lhs.mValue == rhs.mValue; } ///< Compares lhs and rhs. @param lhs a VChar @param rhs a VChar @return true if lhs == rhs
#ifndef VPLATFORM_DISALLOW_VCHAR_OPERATOR_EQUALS_CHAR
inline bool operator==(const VChar& lhs, char rhs) { return lhs.mValue == rhs; }                ///< Compares lhs and rhs. @param lhs a VChar @param rhs a char @return true if lhs == rhs
inline bool operator==(char lhs, const VChar& rhs) { return lhs == rhs.mValue; }                ///< Compares lhs and rhs. @param lhs a char @param rhs a VChar @return true if lhs == rhs
#endif

inline bool operator!=(const VChar& lhs, const VChar& rhs) { return !operator==(lhs, rhs); }    ///< Compares lhs and rhs. @param lhs a VChar @param rhs a VChar @return true if lhs != rhs
#ifndef VPLATFORM_DISALLOW_VCHAR_OPERATOR_EQUALS_CHAR
inline bool operator!=(const VChar& lhs, char rhs) { return !operator==(lhs, rhs); }            ///< Compares lhs and rhs. @param lhs a VChar @param rhs a char @return true if lhs != rhs
inline bool operator!=(char lhs, const VChar& rhs) { return !operator==(lhs, rhs); }            ///< Compares lhs and rhs. @param lhs a char @param rhs a VChar @return true if lhs != rhs
#endif

inline bool operator<(const VChar& lhs, const VChar& rhs) { return lhs.mValue < rhs.mValue; }   ///< Compares lhs and rhs. @param lhs a VChar @param rhs a VChar @return true if lhs < rhs
inline bool operator<(const VChar& lhs, char rhs) { return lhs.mValue < rhs; }                  ///< Compares lhs and rhs. @param lhs a VChar @param rhs a char @return true if lhs < rhs
inline bool operator<(char lhs, const VChar& rhs) { return lhs < rhs.mValue; }                  ///< Compares lhs and rhs. @param lhs a char @param rhs a VChar @return true if lhs < rhs

inline bool operator<=(const VChar& lhs, const VChar& rhs) { return !operator>(lhs, rhs); }     ///< Compares lhs and rhs. @param lhs a VChar @param rhs a VChar @return true if lhs <= rhs
inline bool operator<=(const VChar& lhs, char rhs) { return !operator>(lhs, rhs); }             ///< Compares lhs and rhs. @param lhs a VChar @param rhs a char @return true if lhs <= rhs
inline bool operator<=(char lhs, const VChar& rhs) { return !operator>(lhs, rhs); }             ///< Compares lhs and rhs. @param lhs a char @param rhs a VChar @return true if lhs <= rhs

inline bool operator>=(const VChar& lhs, const VChar& rhs) { return !operator<(lhs, rhs); }     ///< Compares lhs and rhs. @param lhs a VChar @param rhs a VChar @return true if lhs >= rhs
inline bool operator>=(const VChar& lhs, char rhs) { return !operator<(lhs, rhs); }             ///< Compares lhs and rhs. @param lhs a VChar @param rhs a char @return true if lhs >= rhs
inline bool operator>=(char lhs, const VChar& rhs) { return !operator<(lhs, rhs); }             ///< Compares lhs and rhs. @param lhs a char @param rhs a VChar @return true if lhs >= rhs

inline bool operator>(const VChar& lhs, const VChar& rhs) { return operator<(rhs, lhs); }       ///< Compares lhs and rhs. @param lhs a VChar @param rhs a VChar @return true if lhs > rhs
inline bool operator>(const VChar& lhs, char rhs) { return operator<(rhs, lhs); }               ///< Compares lhs and rhs. @param lhs a VChar @param rhs a char @return true if lhs > rhs
inline bool operator>(char lhs, const VChar& rhs) { return operator<(rhs, lhs); }               ///< Compares lhs and rhs. @param lhs a char @param rhs a VChar @return true if lhs > rhs

#endif /* vchar_h */
