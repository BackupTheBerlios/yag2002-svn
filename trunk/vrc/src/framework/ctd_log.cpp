/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2004-2006, Ali Botorabi
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
 # log system
 #
 #   date of creation:  03/06/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_base.h>
#include "ctd_log.h"

using namespace std;

namespace CTD
{

//! This is the default system log instance
Log log;


//! Implementation of logging system
Log::Log() :
_severity( L_DEBUG ),
basic_ostream< char >( &_stream )
{
    _stream.setLog( this );
}

Log::~Log()
{
    // delete all allocated streams, except the std streams
    vector< Sink* >::iterator pp_sink = _sinks.begin(), pp_sinkEnd = _sinks.end();
    for ( ; pp_sink != pp_sinkEnd; pp_sink++ )
        if ( !( *pp_sink )->_stdstream ) delete *pp_sink;
}

void Log::addSink( const std::string& sinkname, const std::string& filename, unsigned int loglevel )
{
    assert( loglevel <= L_ERROR && loglevel >= L_DEBUG );

    // check if there is already a sink with requested sink name
    vector< Sink* >::iterator pp_sink = _sinks.begin(), pp_sinkEnd = _sinks.end();
    for ( ; pp_sink != pp_sinkEnd; pp_sink++ )
        assert( ( *pp_sink )->_name == sinkname && "sink name already exists!" );            

    fstream* p_stream = new fstream;
    p_stream->open( filename.c_str(), ios_base::binary | ios_base::out );
    if ( !*p_stream )
    {   
        delete p_stream;
    }
    else
    {
        Sink *p_sink = new Sink( sinkname, p_stream, loglevel );
        _sinks.push_back( p_sink );
    }
}

void Log::addSink( const string& sinkname, ostream& sink, unsigned int loglevel )
{
    assert( loglevel <= L_ERROR && loglevel >= L_DEBUG );
    
    // check if there is already a sink with requested sink name
    vector< Sink* >::iterator pp_sink = _sinks.begin(), pp_sinkEnd = _sinks.end();
    for ( ; pp_sink != pp_sinkEnd; pp_sink++ )
        assert( ( *pp_sink )->_name != sinkname && "sink name already exists!" );            

    Sink *p_sink = new Sink( sinkname, &sink, loglevel, true );
    _sinks.push_back( p_sink );
}

void Log::removeSink( const string& sinkname )
{
    vector< Sink* >::iterator pp_sink = _sinks.begin(), pp_sinkEnd = _sinks.end();
    for ( ; pp_sink != pp_sinkEnd; pp_sink++ )
    {
        if ( ( *pp_sink )->_name == sinkname )
        {
            delete *pp_sink;
            return;
        }
    }

    assert( NULL && "sink name does not exist!" );
}

void Log::out( const string& msg )
{
    vector< Sink* >::iterator pp_sink = _sinks.begin(), pp_sinkEnd = _sinks.end();
    for ( ; pp_sink != pp_sinkEnd; pp_sink++ )
    {
        if ( ( *pp_sink )->_logLevel < _severity )
        {
            *( ( *pp_sink )->_p_stream ) << msg;
        }
    }
}

void Log::setSeverity( unsigned int severity )
{
    _severity = severity;
}

//---------------------------
Log::LogStreamBuf::LogStreamBuf() :
basic_streambuf< char >()
{
}

Log::LogStreamBuf::~LogStreamBuf()
{
}

void Log::LogStreamBuf::setLog( Log *p_log )
{
    _p_log = p_log;
}

basic_ios< char >::int_type Log::LogStreamBuf::overflow( int_type c )
{
    if( !char_traits< char >::eq_int_type( c, char_traits< char >::eof() ) )
    {
        _msg += c;
        if( c == '\n' )
        {
            _p_log->out( _msg );
            _msg = "";
        }
    }
    return char_traits< char >::not_eof( c );
}

ostream& Log::operator << ( Log::LogLevel& ll )
{
    setSeverity( ll._level );
    return *this;
}

}

