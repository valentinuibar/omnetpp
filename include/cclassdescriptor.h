//==========================================================================
//  CCLASSDESCRIPTOR.H - part of
//                     OMNeT++/OMNEST
//            Discrete System Simulation in C++
//
//
//  Declaration of the following classes:
//    cClassDescriptor  : metainfo about structs and classes
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2008 Andras Varga
  Copyright (C) 2006-2008 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#ifndef __CCLASSDESCRIPTOR_H
#define __CCLASSDESCRIPTOR_H

#include <string>
#include "cownedobject.h"
#include "simtime.h"

NAMESPACE_BEGIN


/**
 * Abstract base class for structure description classes, used mainly
 * with message subclassing.
 *
 * Subclasses of cClassDescriptor encapsulate the kind of reflection
 * information (in the Java sense) which is needed by Tkenv to display
 * fields in a message, struct or object created with the .msg syntax.
 * The cClassDescriptor subclass is generated along with the message class,
 * (struct, object, etc.).
 *
 * When Tkenv encounters a message object, it creates an appropriate
 * cClassDescriptor object and uses that to find out what fields the
 * message object has, what are their values etc. The message object
 * is said to be the `client object' of the cClassDescriptor object.
 *
 * @ingroup Internals
 */
class SIM_API cClassDescriptor : public cNoncopyableOwnedObject
{
  public:
    /// Field types.
    enum {
        FD_ISARRAY = 0x01,    ///< field is an array: int a[]; int a[10];
        FD_ISCOMPOUND = 0x02, ///< basic type (T) is struct or class: T a; T *a; T a[10]; T *a[]
        FD_ISPOINTER = 0x04,  ///< field is pointer or pointer array: T *a; T *a[]; T *a[10];
        FD_ISCOBJECT = 0x08,  ///< if ISCOMPOUND: basic type (T) subclasses from cObject
        FD_ISCOWNEDOBJECT = 0x10, ///< if ISCOMPOUND: basic type (T) subclasses from cOwnedObject
        FD_ISEDITABLE = 0x20, ///< whether field supports setFieldValueAsString()
        FD_NONE = 0x0
    };

  private:
    std::string baseclassname;
    cClassDescriptor *baseclassdesc;
    int inheritancechainlength;
    int extendscobject;

  protected:
    // utility functions for converting from/to strings
    static std::string long2string(long l);
    static long string2long(const char *s);
    static std::string ulong2string(unsigned long l);
    static unsigned long string2ulong(const char *s);
    static std::string int642string(int64 l);
    static int64 string2int64(const char *s);
    static std::string uint642string(uint64 l);
    static uint64 string2uint64(const char *s);
    static std::string bool2string(bool b);
    static bool string2bool(const char *s);
    static std::string double2string(double d);
    static std::string double2string(SimTime t) {return t.str();}
    static double string2double(const char *s);
    static std::string enum2string(long e, const char *enumname);
    static long string2enum(const char *s, const char *enumname);
    static std::string oppstring2string(const char *s) {return s?s:"";}
    static std::string oppstring2string(const opp_string& s) {return s.c_str();}
    static std::string oppstring2string(const std::string& s)  {return s;}
    static void string2oppstring(const char *s, opp_string& str) {str = s?s:"";}
    static void string2oppstring(const char *s, std::string& str) {str = s?s:"";}
    static const char **mergeLists(const char **list1, const char **list2);

  public:
    /** @name Constructors, destructor, assignment. */
    //@{

    /**
     * Constructor.
     */
    cClassDescriptor(const char *classname, const char *_baseclassname=NULL);

    /**
     * Destructor.
     */
    virtual ~cClassDescriptor();
    //@}

    /** @name Getting descriptor for an object or a struct. */
    //@{

    /**
     * Returns the descriptor object for the given class. The returned
     * descriptor object must not be deleted.
     */
    static cClassDescriptor *getDescriptorFor(const char *classname);

    /**
     * Returns the descriptor object for the given object. This can return
     * descriptor for a base class, if there is no exact match.
     * The returned descriptor object must not be deleted.
     */
    static cClassDescriptor *getDescriptorFor(cObject *object);
    //@}

    /** @name Querying and setting fields of the client object. */
    //@{

    /**
     * Returns true if this descriptor supports the given object's class.
     * If obj can be cast (dynamic_cast) to the class the descriptor supports,
     * the method should return true.
     */
    virtual bool doesSupport(cObject *obj) const {return false;}

    /**
     * Returns the descriptor for the base class, if available.
     */
    virtual cClassDescriptor *getBaseClassDescriptor() const;

    /**
     * Returns true if cObject's class descriptor is present on the inheritance chain.
     */
    bool extendsCObject() const;

    /**
     * Returns the number of base classes up to the root, as reflected in the
     * descriptors (see getBaseClassDescriptor()).
     */
    int getInheritanceChainLength() const;

    /**
     * Returns a NULL-terminated string array with the names of properties on this
     * class. The list includes the properties of the ancestor classes as well.
     */
    virtual const char **getPropertyNames() const = 0;

    /**
     * Returns the value of the given property of the descriptor as a single string.
     * Returns NULL if there is no such property. For structured property values
     * (with multiple keys and/or list(s) inside), the value is returned as a
     * single unparsed string. If a property appears in a base class and its
     * subclass too, the subclass's property will be returned.
     */
    virtual const char *getProperty(const char *propertyname) const = 0;

    /**
     * Returns the number of fields in the described class.
     */
    virtual int getFieldCount() const = 0;

    /**
     * Returns the name of a field in the described class.
     * The argument must be in the 0..getFieldCount()-1 range, inclusive.
     */
    virtual const char *getFieldName(int field) const = 0;

    /**
     * Returns the index of the field with the given name, or -1 if not found.
     * cClassDescriptor provides an default implementation, but it is
     * recommended to replace it in subclasses with a more efficient version.
     */
    virtual int findField(const char *fieldName) const;

    /**
     * Returns the type flags of a field in the described class. Flags is a
     * binary OR of the following: FD_ISARRAY, FD_ISCOMPOUND, FD_ISPOINTER,
     * FD_ISCOWNEDOBJECT, FD_ISCOBJECT.
     * The argument must be in the 0..getFieldCount()-1 range, inclusive.
     */
    virtual unsigned int getFieldTypeFlags(int field) const = 0;

    /** @name Utility functions based on getFieldTypeFlags() */
    //@{
    bool getFieldIsArray(int field) const {return getFieldTypeFlags(field) & FD_ISARRAY;}
    bool getFieldIsCompound(int field) const {return getFieldTypeFlags(field) & FD_ISCOMPOUND;}
    bool getFieldIsPointer(int field) const {return getFieldTypeFlags(field) & FD_ISPOINTER;}
    bool getFieldIsCObject(int field) const {return getFieldTypeFlags(field) & (FD_ISCOBJECT|FD_ISCOWNEDOBJECT);}
    bool getFieldIsCOwnedObject(int field) const {return getFieldTypeFlags(field) & FD_ISCOWNEDOBJECT;}
    bool getFieldIsEditable(int field) const {return getFieldTypeFlags(field) & FD_ISEDITABLE;}
    //@}

    /**
     * Returns the name of the class on which the given field was declared.
     */
    virtual const char *getFieldDeclaredOn(int field) const;

    /**
     * Returns the declared type of a field in the described class as a string.
     * The argument must be in the 0..getFieldCount()-1 range, inclusive.
     */
    virtual const char *getFieldTypeString(int field) const = 0;

    /**
     * Returns a NULL-terminated string array with the names of the given
     * field's properties.
     */
    virtual const char **getFieldPropertyNames(int field) const = 0;

    /**
     * Returns the value of the given property of the given field in the
     * described class as a single string. Returns NULL if there is no such
     * property. For structured property values (with multiple keys and/or
     * list(s) inside), the value is returned as a single unparsed string.
     */
    virtual const char *getFieldProperty(int field, const char *propertyname) const = 0;

    /**
     * Returns the array size of a field in the given object. If the field is
     * not an array, it returns 0.
     */
    virtual int getFieldArraySize(void *object, int field) const = 0;

    /**
     * Returns the value of the given field in the given object as a string.
     * For compound fields, the message compiler generates code which calls operator<<.
     */
    virtual std::string getFieldValueAsString(void *object, int field, int i) const = 0;

    /**
     * Sets the value of a field in the given object by parsing the given value string.
     * Returns true if successful, and false if an error occurred or the field
     * does not support setting.
     */
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const = 0;

    /**
     * Returns the declared type name of a compound field in the described class.
     * The field is a pointer, the "*" is removed. The return value may be used
     * as class name to obtain a class descriptor for this compound field.
     */
    virtual const char *getFieldStructName(int field) const = 0;

    /**
     * Returns the pointer to the value of a compound field in the given object.
     */
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const = 0;
    //@}

    /** @name Functions for backwards compatibility (OMNeT++ 4.2 and earlier). */
    //@{
    _OPPDEPRECATED virtual int getFieldCount(void *UNUSED) const  {return getFieldCount();}
    _OPPDEPRECATED virtual const char *getFieldName(void *UNUSED, int field) const  {return getFieldName(field);}
    _OPPDEPRECATED virtual int findField(void *UNUSED, const char *fieldName) const {return findField(fieldName);}
    _OPPDEPRECATED virtual unsigned int getFieldTypeFlags(void *UNUSED, int field) const  {return getFieldTypeFlags(field);}
    _OPPDEPRECATED virtual bool getFieldIsArray(void *UNUSED, int field) const  {return getFieldIsArray(field);}
    _OPPDEPRECATED virtual bool getFieldIsCompound(void *UNUSED, int field) const  {return getFieldIsCompound(field);}
    _OPPDEPRECATED virtual bool getFieldIsPointer(void *UNUSED, int field) const  {return getFieldIsPointer(field);}
    _OPPDEPRECATED virtual bool getFieldIsCObject(void *UNUSED, int field) const  {return getFieldIsCObject(field);}
    _OPPDEPRECATED virtual bool getFieldIsCOwnedObject(void *UNUSED, int field) const  {return getFieldIsCOwnedObject(field);}
    _OPPDEPRECATED virtual bool getFieldIsEditable(void *UNUSED, int field) const  {return getFieldIsEditable(field);}
    _OPPDEPRECATED virtual const char *getFieldDeclaredOn(void *UNUSED, int field) const  {return getFieldDeclaredOn(field);}
    _OPPDEPRECATED virtual const char *getFieldTypeString(void *UNUSED, int field) const  {return getFieldTypeString(field);}
    _OPPDEPRECATED virtual const char *getFieldProperty(void *UNUSED, int field, const char *propertyname) const  {return getFieldProperty(field, propertyname);}
    _OPPDEPRECATED virtual int getArraySize(void *object, int field) const  {return getFieldArraySize(object, field);}
    _OPPDEPRECATED virtual std::string getFieldAsString(void *object, int field, int i) const  {return getFieldValueAsString(object, field, i);}
    _OPPDEPRECATED virtual bool getFieldAsString(void *object, int field, int i, char *buf, int bufsize) const;
    _OPPDEPRECATED virtual bool setFieldAsString(void *object, int field, int i, const char *value) const  {return setFieldValueAsString(object, field, i, value);}
    _OPPDEPRECATED virtual const char *getFieldStructName(void *UNUSED, int field) const  {return getFieldStructName(field);}
    _OPPDEPRECATED virtual void *getFieldStructPointer(void *object, int field, int i) const  {return getFieldStructValuePointer(object, field, i);}
    //@}
};

NAMESPACE_END


#endif


