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
 #
 #   date of creation:  07/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _SMARTPTR_H_
#error "don't include this file directly, include smartptr.h instead!"
#endif

template< class Type >
SmartPtr< Type >::SmartPtr() :
 _p_ref( NULL )
{
}

template< class Type >
SmartPtr< Type >::SmartPtr( const SmartPtr& copy ) :
 _p_ref( copy.getRef() )
{
    if ( _p_ref )
        _p_ref->increaseRefCnt();
}

template< class Type >
SmartPtr< Type >::SmartPtr( Type* p_ref ) :
 _p_ref( p_ref )
{
    if ( _p_ref )
        _p_ref->increaseRefCnt();
}

template< class Type >
SmartPtr< Type >::~SmartPtr()
{
    if ( _p_ref )
    {
        _p_ref->decreaseRefCnt();
        if ( !_p_ref->getCount() )
        {
            delete _p_ref;
            _p_ref = NULL;
        }
    }
}

template< class Type >
Type* SmartPtr< Type >::getRef() const
{
    return _p_ref;
}

template< class Type >
bool SmartPtr< Type >::valid() const
{
    return ( _p_ref != NULL );
}

template< class Type >
Type& SmartPtr< Type >::operator * ()
{
    return *_p_ref;
}

template< class Type >
Type& SmartPtr< Type >::operator * () const
{
    return *_p_ref;
}

template< class Type >
Type* SmartPtr< Type >::operator -> ()
{
    return _p_ref;
}

template< class Type >
Type* SmartPtr< Type >::operator -> () const
{
    return _p_ref;
}

template< class Type >
SmartPtr< Type >& SmartPtr< Type >::operator = ( Type* p_right )
{
    if ( _p_ref == p_right )
        return *this;

    // check for NULL assignment
    if ( !p_right )
    {
        if ( _p_ref )
        {
            _p_ref->decreaseRefCnt();
            if ( !_p_ref->getCount() )
            {
                delete _p_ref;
                _p_ref = NULL;
            }
            _p_ref = p_right;
        }
    }
    else
    {
        if ( _p_ref )
        {
            _p_ref->decreaseRefCnt();
            if ( !_p_ref->getCount() )
            {
                delete _p_ref;
                _p_ref = NULL;
            }
        }

        _p_ref = p_right;
        _p_ref->increaseRefCnt();
    }

    return *this;
}

template< class Type >
SmartPtr< Type >& SmartPtr< Type >::operator = ( const SmartPtr< Type >& right )
{
    // check self assignment
    if ( _p_ref == right.getRef() )
        return *this;

    if ( _p_ref )
    {
        _p_ref->decreaseRefCnt();
        if ( !_p_ref->getCount() )
        {
            delete _p_ref;
            _p_ref = NULL;
        }
    }

    if ( right.getRef() )
    {
        right->increaseRefCnt();
        _p_ref = right.getRef();
    }

    return *this;
}
