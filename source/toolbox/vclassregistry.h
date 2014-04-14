/*
Copyright c1997-2014 Trygve Isaacson. All rights reserved.
This file is part of the Code Vault version 4.1
http://www.bombaydigital.com/
*/

#ifndef vclassregistry_h
#define vclassregistry_h

/** @file */

#include "vstring.h"

class VClassFactory;
typedef std::vector<const VClassFactory*> VClassFactoryPtrVector;    ///< A vector of pointers to VClassFactory objects.

/**
    @ingroup toolbox
*/

/**
VClassRegistry provides a mechanism for instantiating objects dynamically
at runtime based on class IDs. This allows you to write code that obtains
a class "ID" and then calls the registry to instantiate the corresponding
type of object. You decide what an ID is (it's just a string, so the simple
thing is to use the class name as the ID, perhaps qualified with something
akin to a Java package name to ensure uniqueness across a global name space),
and provide the factory function.

If you prefer, you can create multiple registries and register your
class factories with them as you see fit. However, the easiest way to use
the registry is to just use the singleton that it creates automatically.

To do this, all you need to do is define a class factory for each registered
class. Two macros are provide to make this trivial to add in a consistent
fashion. Suppose you have class Foo for which you want a factory
registered with the global registry:

In foo.h:

    class Foo { .... };

    DEFINE_CLASSFACTORY(Foo);

In foo.cpp:

    DECLARE_CLASSFACTORY(Foo);

When you need a Foo object but don't know the actual type until runtime
(let's say the string <code>name</code> is given to you during some
kind of deserialization process where everything is a subclass of Foo, and all
of the Foo subclasses also have class factory macros) you just do this:

    Foo* aFoo =
        (Foo*) VClassRegistry::registry()->instantiateObject(name);

The purpose of the above is to create a single FooFactory as a static
class variable, which will be created automatically at program startup,
that when constructed registers itself with the global registry. This
means that you don't have to explicitly register all your classes at
some central location in your code; it's almost automatic just by
defining the factory that it will get registered.

The method VClassRegistry::registry() is provided to ensure that when
the first attempt is made to register a class -- which will presumably
be at static initialization time -- the global registry will get
created exactly once, before it is actually accessed. This is because
the order of static initialization is not guaranteed, and we don't want
to clutter the caller's code with checks for the global variable being
non-NULL. This is both cleaner and fool-proof.

If you want a separate registry for a set of classes, just create your
own separate VClassFactory object, and an accessor function that makes
sure it's created upon first use, just like VClassRegistry::registry().
In other words, define your own "myRegistry()" function that looks
exactly like the code in VClassRegistry::registry() but uses your own
registry global rather than VClassRegistry::gRegistry.

In the class declaration of the class that will own your registry:

    public:
        static VClassRegistry* myRegistry();
    protected:
        static VClassRegistry* gRegistry;

In the class implementation:

    // static
    VClassRegistry* ThingThatOwnsMyRegistry::gRegistry = NULL;

    // static
    VClassRegistry* ThingThatOwnsMyRegistry::registry()
        {
        // We're using the "create on first use" idiom here.
        if (gRegistry == NULL)
            gRegistry = new VClassRegistry();

        return gRegistry;
        }

Then your FooFactory would only have to change its constructor from
the above example to:

    FooFactory(const VString& classID) : VClassFactory(classID)
        { ThingThatOwnsMyRegistry::registry()->registerClass(this); }

*/
class VClassRegistry {
    public:

        /**
        Returns a pointer to the global class registry object.
        */
        static VClassRegistry* registry();

        /**
        Constructs the registry.
        */
        VClassRegistry();
        /**
        Destructor.
        */
        virtual ~VClassRegistry();

        /**
        Instantiates and returns an object of the correct type for the
        specified class ID. Throws an exception if there is no factory
        registered for that class ID. The return type is void* because
        only the factory function knows the actual type; the caller
        also needs to know something such as the base class, or use RTTI
        in some fashion, to get the correct type.
        @param    classID    the class ID of the class to be instantiated
        @return the newly instantiated object
        */
        void* instantiateObject(const VString& classID) const;
        /**
        Registers a class factory.
        @param factory the factory for creating some class
        */
        void registerClass(const VClassFactory* factory);
        /**
        Returns the class factory for a specified class ID. Useful if you
        can optimize performance by getting the factory once and using it
        to create many objects, rather than looking it up each time. Throws
        an exception if there is no factory registered for that class ID.
        @param    classID    the class ID of the class whose factory is to be found
        @return the factory for the specified class ID
        */
        const VClassFactory* findClassFactory(const VString& classID) const;

    private:

        VClassFactoryPtrVector mFactories; ///< The factories that have been registered.

        static VClassRegistry* gRegistry;  ///< The global registry we maintain.
};

/**
VClassFactory is the abstract base class that you subclass to provide
a method that instantiates a particular type of object. You need only
provide an override of the pure virtual function instaniateObject().
However, it is useful to use the example of the FooFactory class
above as the simplest way to get the factory registered automatically
with a VClassRegistry at static initialization time.
*/
class VClassFactory {
    public:

        /**
        Constructs the factory.
        @param    classID    the string that uniquely identifies the class
        */
        VClassFactory(const VString& classID);
        /**
        Destructor.
        */
        virtual ~VClassFactory() {}

        /**
        Pure virtual method (must be overridden) that instantiates and
        returns an object of the appropriate type for this factory.
        @return the newly instantiated object
        */
        virtual void* instantiateObject() const = 0;
        /**
        Tests to see if this factory is the one that knows how to
        instantiate a particular class ID.
        @param    classID    the class ID to test
        @return true if this is the factory for that class ID
        */
        bool matchesClassID(const VString& classID) const;
        /**
        Returns the class ID of this factory (useful for debugging).
        @return the class ID of this factory
        */
        void getClassID(VString& classID) const;

    private:

        VString mClassID;    ///< The class ID this factory is for.
};

/**
@def DEFINE_CLASSFACTORY(classname, factoryname)
DEFINE_CLASSFACTORY is a macro that you should use to define a VClassFactory subclass
for a given class. The definition of such a class is boilerplate, and this macro
defines it correctly. Typically, you'll say something like:

- DEFINE_CLASSFACTORY(Foo, FooFactory);
*/
#define DEFINE_CLASSFACTORY(classname, factoryname) \
class factoryname : public VClassFactory { \
    public: \
 \
        static factoryname gFactory; \
 \
        factoryname(const VString& classID) \
            : VClassFactory(classID) \
            { \
            VClassRegistry::registry()->registerClass(this); \
        } \
 \
        virtual ~factoryname() {} \
 \
        virtual void* instantiateObject() const { return new classname(); } \
 \
}

/**
@def DECLARE_CLASSFACTORY(classname, factoryname)
DECLARE_CLASSFACTORY is a macro that lets you declare a VClassFactory subclass,
typically in your implementation file. Typically, you'll say something like:

- DECLARE_CLASSFACTORY(Foo, FooFactory);
*/
#define DECLARE_CLASSFACTORY(classname, factoryname) factoryname factoryname::gFactory(#classname)

#endif /* vclassregistry_h */

