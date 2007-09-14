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
 # log system
 #
 #   date of creation:  03/06/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>
#include "log.h"

namespace yaf3d
{

//! This is the default system log instance
Log yaf3dlog;


//! Implementation of logging system
Log::Log() :
std::basic_ostream< char >( &_stream ),
_severity( L_DEBUG ),
_printSeverityLevel( true )
{
    _stream.setLog( this );
}

Log::~Log()
{
    // delete all allocated streams, except the std streams
    std::vector< Sink* >::iterator p_sink = _sinks.begin(), p_sinkEnd = _sinks.end();
    for ( ; p_sink != p_sinkEnd; p_sink++ )
        delete *p_sink;
}

bool Log::addSink( const std::string& sinkname, const std::string& filename, unsigned int loglevel )
{
    if( loglevel > L_INFO || loglevel < L_VERBOSE )
    {
        log_error << "invalid log level for given sink '"  << sinkname <<  "'" << std::endl;
        return false;
    }

    // check if there is already a sink with requested sink name
    std::vector< Sink* >::iterator p_sink = _sinks.begin(), p_sinkEnd = _sinks.end();
    for ( ; p_sink != p_sinkEnd; p_sink++ )
    {
        if ( ( *p_sink )->_name == sinkname )
        {
            log_error << "sink name '" << sinkname << "' already exists!" << std::endl;
            return false;
        }
    }

    std::fstream* p_stream = new std::fstream;
    p_stream->open( filename.c_str(), std::ios_base::binary | std::ios_base::out );
    if ( !*p_stream )
    {   
        delete p_stream;
        log_error << "cannot open log file '" << filename << "' for sink '" << sinkname << "'" << std::endl;
        return false;

    }
    else
    {
        Sink *p_sink = new Sink( sinkname, p_stream, loglevel );
        _sinks.push_back( p_sink );
    }

    return true;
}

bool Log::addSink( const std::string& sinkname, std::ostream& sink, unsigned int loglevel )
{
    if( loglevel > L_INFO || loglevel < L_VERBOSE )
    {
        log_error << "invalid log level for given sink '" << sinkname <<  "'" << std::endl;
        return false;
    }
    
    // check if there is already a sink with requested sink name
    std::vector< Sink* >::iterator p_beg = _sinks.begin(), p_end = _sinks.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        if ( ( *p_beg )->_name == sinkname )
        {
            log_error << "sink name '" << sinkname << "' already exists!"  << std::endl;
            return false;
        }
    }

    Sink* p_sink = new Sink( sinkname, &sink, loglevel, true );
    _sinks.push_back( p_sink );

    return true;
}

void Log::removeSink( const std::string& sinkname )
{
    std::vector< Sink* >::iterator p_sink = _sinks.begin(), p_sinkEnd = _sinks.end();
    for ( ; p_sink != p_sinkEnd; ++p_sink )
    {
        if ( ( *p_sink )->_name == sinkname )
        {
            delete *p_sink;
            _sinks.erase( p_sink );
            return;
        }
    }

    assert( NULL && "sink name does not exist!" );
}

void Log::enableSeverityLevelPrinting( bool en )
{
    _printSeverityLevel = en;
}

void Log::out( const std::string& msg )
{
    std::vector< Sink* >::iterator p_sink = _sinks.begin(), p_sinkEnd = _sinks.end();
    for ( ; p_sink != p_sinkEnd; p_sink++ )
    {
        if ( ( *p_sink )->_logLevel <= _severity )
        {
            *( ( *p_sink )->_p_stream ) << msg;
            ( ( *p_sink )->_p_stream )->flush();
        }
    }
}

void Log::setSeverity( unsigned int severity )
{
    _severity = severity;
}

//---------------------------
Log::LogStreamBuf::LogStreamBuf() :
std::basic_streambuf< char >(),
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

std::basic_ios< char >::int_type Log::LogStreamBuf::overflow( int_type c )
{
    if( !std::char_traits< char >::eq_int_type( c, std::char_traits< char >::eof() ) )
    {
        _msg += c;
        if( c == '\n' )
        {
            std::string severity;
            if ( _p_log->_printSeverityLevel )
            {
                switch ( _p_log->_severity )
                {
                case L_INFO:
                    severity = "[info]    ";
                    break;
                case L_DEBUG:
                    severity = "[debug]   ";
                    break;
                case L_WARNING:
                    severity = "[warning] ";
                    break;
                case L_ERROR:
                    severity = "[error]   ";
                    break;
                case L_VERBOSE:
                    severity = "[verbose] ";
                    break;
                default:
                    assert( NULL && "unknown log severity" );
                }
            }

            // add a carriage return to end of line
            _msg[ _msg.length() - 1 ] = '\r';
            _msg += "\n";
            _p_log->out( severity + _msg );
            _msg = "";
        }
    }
    return std::char_traits< char >::not_eof( c );
}

std::ostream& Log::operator << ( const Log::LogLevel& ll )
{
    setSeverity( ll._level );    
    return *this;
}

} // namespace yaf3d

