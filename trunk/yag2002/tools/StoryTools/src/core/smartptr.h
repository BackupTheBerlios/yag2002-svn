/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2007, A. Botorabi
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the Free
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 ****************************************************************/

/*###############################################################
 # simple non-threadsafe, ref-counted smart pointer
 #  for more details see below.
 #
 #   date of creation:  07/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _SMARTPTR_H_
#define _SMARTPTR_H_

#include <main.h>

namespace beditor
{

//! Convenient macro for setting up the smart pointer access, place it into derived class
#define DECLARE_SMARTPTR_ACCESS( classname )    friend class SmartPtr< classname >;

//! Class for describing a link between two nodes
template< class Type >
class SmartPtr
{
    public:

                                     SmartPtr();

                                     SmartPtr( const SmartPtr& copy );

        explicit                     SmartPtr( Type* p_ref );

        virtual                      ~SmartPtr();

        //! Return the reference pointer with ref count modifications
        Type*                        getRef() const;

        //! Pointer access operator
        Type*                        operator -> ();

        //! Pointer access operator
        Type*                        operator -> () const;

        //! Dereference operator
        Type&                        operator * ();

        //! Dereference operator
        Type&                        operator * () const;

        //! Assigment operator for ref type pointer to smart pointer
        SmartPtr&                    operator = ( Type* p_right );

        //! Assigment operator for smart pointer to smart pointer
        SmartPtr&                    operator = ( const SmartPtr& right );

    protected:

        Type*                        _p_ref;
};

//! Ref counter class used by classes as base which derive from SmartPtr
template< class Type >
class RefCount
{
    public:

                                    RefCount() :
                                     _refCount( 0 )
                                    {
                                    }

    public:

        virtual                     ~RefCount() {}

        unsigned int                getCount() const { return _refCount; }

        void                        increaseRefCnt() { ++_refCount; }

        void                        decreaseRefCnt() { --_refCount; }

        unsigned int                _refCount;

    friend class SmartPtr< Type >;
};

#include "smartptr.inl"

} // namespace beditor


/**

Example for using the smart pointer class.
------------------------------------------


// class declaration, derive the class from RefCount
class MyClass : public RefCount< MyClass >
{
    //! Set the smart pointer class as friend
    DECLARE_SMARTPTR_ACCESS( MyClass )

    public:

                                        MyClass();

    protected:

        //! Make the destructor protected in order to avoid manaual deletion by user.
        //! The deletion of an object is controlled by the smart pointer.
        virtual                         ~MyClass();
};

// later in code
SmartPtr< MyClass > testSmartPtr()
{
    SmartPtr< MyClass > smtprtA;                    // smtprtA is constructed and has an empty reference
    SmartPtr< MyClass > smtprtB( new MyClass );     // smtprtB is constructed and initialized with a reference ( count 1 )

    smtprtA = new MyClass;                          // smtprtA gets a referrence
    smtprtA = smtprtB;                              // smtprtA deletes its old reference ( as ref count to zero because of the assignment ) and gets a new reference ( that of smtprtB )

    // automatic ref counting when smart pointer object get destroyed
    {
        SmartPtr< MyClass > smtprtC;
        smtprtC = smtprtA;                          // now smtprtC, smtprtB, and smtprtA have all ref count 3 as they all hold the same reference
    }
    // at this point smtprtA and smtprtB get ref count of 2 as smtprtC is destroyed

    // now assing a NULL to smtprtA
    smtprtB = NULL;                                 // here smtprtB gets an empty ref and smtprtB gets a ref count of 1

    // return the smart pointer, return value can be assigned to another smart pointer object
    return smtprtA;

} // on function return the remaining reference in smtprtA gets deteted


// now call the test function some where

    ...
    SmartPtr< MyClass > smtprt = testSmartPtr(); // now smtprt will get the reference carried out by smtprtA in the test function and will have the ref count of 1
    ...


*/

#endif // _SMARTPTR_H_
