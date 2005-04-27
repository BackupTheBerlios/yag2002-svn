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
 # base class for singletons
 #
 #   date of creation:  02/17/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/


template< class TypeT >
Singleton< TypeT >::Singleton()
{
}

template< class TypeT >
Singleton< TypeT >::~Singleton()
{
}

template< class TypeT >
TypeT* Singleton< TypeT >::get()
{
    if ( !_p_instance )
        return _p_instance = new TypeT;

    return _p_instance;
}

template< class TypeT >
void Singleton< TypeT >::destroy()
{
    if ( _p_instance )
        delete _p_instance;

    _p_instance = 0;
}

