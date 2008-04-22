/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2006, A. Botorabi
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
 # base class for singletons
 #
 #   date of creation:  02/17/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _SINGLETON_H_
#define _SINGLETON_H_

//! Convenient macro for implementing a singleton
#define YAF3D_SINGLETON_IMPL( Type ) template<> Type* yaf3d::Singleton< Type >::_p_instance = 0;

namespace yaf3d
{

//! Inherit from this class for your singletons ( phoenix type, single threaded singleton )
template< class TypeT >
class Singleton
{
    public:        

        static inline TypeT*                get();

    protected:

                                            Singleton();

        virtual                             ~Singleton();

        static TypeT*                       _p_instance;

        void                                destroy();

    private:

        //! Avoid copy constructor
                                            Singleton( const Singleton& );
 
        //! Avoid assignment operator
        Singleton&                          operator = ( const Singleton& );
};

#include "singleton.inl"

}

#endif // _SINGLETON_H_
