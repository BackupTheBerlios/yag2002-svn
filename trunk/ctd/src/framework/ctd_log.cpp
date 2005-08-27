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
basic_ostream< char >( &_stream ),
_severity( L_DEBUG ),
_printSeverityLevel( true )
{
    _stream.setLog( this );
}

Log::~Log()
{
    // delete all allocated streams, except the std streams
    vector< Sink* >::iterator pp_sink = _sinks.begin(), pp_sinkEnd = _sinks.end();
    for ( ; pp_sink != pp_sinkEnd; pp_sink++ )
        delete *pp_sink;
}

bool Log::addSink( const std::string& sinkname, const std::string& filename, unsigned int loglevel )
{
    if( loglevel > L_INFO || loglevel < L_ERROR )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "invalid log level for given sink '"  << sinkname <<  "'" << endl;
        return false;
    }

    // check if there is already a sink with requested sink name
    vector< Sink* >::iterator pp_sink = _sinks.begin(), pp_sinkEnd = _sinks.end();
    for ( ; pp_sink != pp_sinkEnd; pp_sink++ )
    {
        if ( ( *pp_sink )->_name == sinkname )
        {
            log << Log::LogLevel( Log::L_ERROR ) << "sink name '" << sinkname << "' already exists!" << endl;
            return false;
        }
    }

    fstream* p_stream = new fstream;
    p_stream->open( filename.c_str(), ios_base::binary | ios_base::out );
    if ( !*p_stream )
    {   
        delete p_stream;
        log << Log::LogLevel( Log::L_ERROR ) << "cannot open log file '" << filename << "' for sink '" << sinkname << "'" << endl;
        return false;

    }
    else
    {
        Sink *p_sink = new Sink( sinkname, p_stream, loglevel );
        _sinks.push_back( p_sink );
    }

    return true;
}

bool Log::addSink( const string& sinkname, ostream& sink, unsigned int loglevel )
{
    if( loglevel > L_INFO || loglevel < L_ERROR )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "invalid log level for given sink '" << sinkname <<  "'" << endl;
        return false;
    }
    
    // check if there is already a sink with requested sink name
    vector< Sink* >::iterator pp_sink = _sinks.begin(), pp_sinkEnd = _sinks.end();
    for ( ; pp_sink != pp_sinkEnd; pp_sink++ )
    {
        if ( ( *pp_sink )->_name == sinkname )
        {
            log << Log::LogLevel( Log::L_ERROR ) << "sink name '" << sinkname << "' already exists!"  << endl;
            return false;
        }
    }

    Sink *p_sink = new Sink( sinkname, &sink, loglevel, true );
    _sinks.push_back( p_sink );

    return true;
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

void Log::enableSeverityLevelPrinting( bool en )
{
    _printSeverityLevel = en;
}

void Log::out( const string& msg )
{
    vector< Sink* >::iterator pp_sink = _sinks.begin(), pp_sinkEnd = _sinks.end();
    for ( ; pp_sink != pp_sinkEnd; pp_sink++ )
    {
        if ( ( *pp_sink )->_logLevel <= _severity )
        {
            *( ( *pp_sink )->_p_stream ) << msg;
            ( ( *pp_sink )->_p_stream )->flush();
        }
    }
}

void Log::setSeverity( unsigned int severity )
{
    _severity = severity;

    if ( !_printSeverityLevel )
        return;

    switch ( severity )
    {
        case L_INFO:
            out( "info:    " );
            break;
        case L_DEBUG:
            out( "debug:   " );
            break;
        case L_WARNING:
            out( "warning: " );
            break;
        case L_ERROR:
            out( "error:   " );
            break;
        default:
            assert( NULL && "unknown log severity" );
    }
}

//---------------------------
Log::LogStreamBuf::LogStreamBuf() :
basic_streambuf< char >(),
_p_log ( NULL )
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

ostream& Log::operator << ( const Log::LogLevel& ll )
{
    setSeverity( ll._level );    
    return *this;
}

}

