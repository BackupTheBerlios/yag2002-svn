/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2002-2004, Ali Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU General Public License 
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public 
 *  License along with this program; if not, write to the Free 
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 * 
 ****************************************************************/

/*###############################################################
 # networking utilities
 #
 #
 #   date of creation:  10/09/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  10/09/2004 boto       creation of RingBuffer
 #
 ################################################################*/

#ifndef _CTD_NETWORKINGUTILS_H_
#define _CTD_NETWORKINGUTILS_H_


#include "base.h"


namespace CTD {


//! Simple ring buffer class holding objects of type TypeT with a depth of BufferDepthT
//!  note: this class is optimized for primitive types ( i.e. TypeT should be bool, float, int, etc.  )
template< typename TypeT, unsigned int BufferDepthT >
class RingBuffer
{

    public:
                                                    RingBuffer()
                                                    {
                                                        m_bRingBufferFilled     = false;
                                                        m_uiRingBufferMarker    = 0;
                                                    }

        void                                        Clear()
                                                    {
                                                        memset( m_aElems, 0, BufferDepthT * sizeof( TypeT ) );
                                                        m_uiRingBufferMarker    = 0;
                                                        m_bRingBufferFilled     = false;
                                                    }

        void                                        AddElement( TypeT value )
                                                    {
                                                        if ( m_uiRingBufferMarker > ( unsigned int )( BufferDepthT - 1 ) ) {
                                                            m_uiRingBufferMarker = 0;
                                                            m_bRingBufferFilled  = true;
                                                        }
                                                        m_aElems[ m_uiRingBufferMarker ] = value;
                                                        m_uiRingBufferMarker++;
                                                    }

        unsigned int                                FillCount()
                                                    {
                                                        return m_bRingBufferFilled ? ( BufferDepthT ): m_uiRingBufferMarker;
                                                    }

        TypeT                                       GetElement( unsigned int uiIndex )
                                                    {
                                                        return m_aElems[ uiIndex ];
                                                    }

    protected:

        TypeT                                       m_aElems[ BufferDepthT ];

        unsigned int                                m_uiRingBufferMarker;

        bool                                        m_bRingBufferFilled;

};

//! This class calculates the avarage update period of incomming net pakets
template < unsigned int HistoryDepthT >
class AvarageUpdatePeriod {

    public:

        //! construct the avarage builder
                                                    AvarageUpdatePeriod() 
                                                    {
                                                        m_fAvarageUpdatePeriod  = 0;
                                                        m_kRingBuffer.Clear();
                                                        m_kTimer.Reset();
                                                    }

        //! add a new incomming update, call this whenever a new update arrives
        void                                        AddUpdate()
                                                    {
                                                        m_kRingBuffer.AddElement( m_kTimer.GetDeltaTime( true ) );
                                                    }

        //! get avarage update period                                                 
        float                                       GetAvarageUpdatePeriod() 
                                                    {
                                                        unsigned int uiFillCount = m_kRingBuffer.FillCount();

                                                        m_fAvarageUpdatePeriod = 0;
                                                        for ( unsigned int uiCnt = 0; uiCnt < uiFillCount; uiCnt++ ) {

                                                            m_fAvarageUpdatePeriod += m_kRingBuffer.GetElement( uiCnt );
                                                        }
                                                        if ( uiFillCount == 0 ) {
                                                            return m_fAvarageUpdatePeriod;
                                                        }
                                                        return m_fAvarageUpdatePeriod / ( float )uiFillCount;
                                                    }

        //! reset the avarage builder
        void                                        Reset()
                                                    {
                                                        m_kTimer.Reset();
                                                        m_fAvarageUpdatePeriod  = 0;
                                                        m_kRingBuffer.Clear();
                                                    }

    protected:

        RingBuffer< float, HistoryDepthT >          m_kRingBuffer;

        NeoEngine::Timer                            m_kTimer;

        float                                       m_fAvarageUpdatePeriod;


};


} // namespace CTD

#endif //_CTD_NETWORKINGUTILS_H_
