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
 # log system
 #
 #   date of creation:  03/06/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_LOG_H_
#define _CTD_LOG_H_

#include <ctd_base.h>
#include <ctd_singleton.h>

namespace CTD
{

class Log;

//! This is the default system log instance
/** Usage:
*            log << Log::LogLevel( Log::L_INFO ) << " this is an info log" << endl;
*            log << " number: " << num << " string: " << mystring << endl;
*/
extern Log log;

//! Class for logging messages to defined sinks
class Log : public std::basic_ostream< char >
{
    public:

        //! Logging thresholds
        enum Level {
            L_ERROR   = 0x4,
            L_WARNING = 0x3,
            L_INFO    = 0x2,
            L_DEBUG   = 0x1
        };

                                                Log();

        virtual                                 ~Log();

        //! Add a file sink
        void                                    addSink( const std::string& sinkname, const std::string& filename, unsigned int loglevel = Level::L_DEBUG );

        //! Add standard sink such as cout
        void                                    addSink( const std::string& sinkname, std::ostream& sink = std::cout, unsigned int loglevel = Level::L_DEBUG );

        //! Remove a sink given its name
        void                                    removeSink( const std::string& sinkname );

        //! Output a message on sinks with given severity
        void                                    out( const std::string& msg );

        //! Set current message severity
        void                                    setSeverity( unsigned int severity );

        //! Struct for setting new loglevel via stream operator <<
        struct LogLevel
        {
                LogLevel( Level threshold ) : _level( threshold ) {}

                Level     _level;
        };

        //! Stream operator for setting current logging severity
        std::ostream&                           operator << ( Log::LogLevel& ll );

    protected:

        //! Avoid copy constructor
        Log&                                    operator = ( const Log& log );

        // Currently set severity
        unsigned int                            _severity;

        //! Log sink 
        struct Sink
        {
                                                Sink( const std::string& name, std::ostream* p_stream, unsigned int loglevel, bool stdstream = false ) :
                                                    _p_stream( p_stream ), 
                                                    _logLevel( loglevel ),
                                                    _name( name ),
                                                    _stdstream( stdstream )
                                                {}
                                                
                                                ~Sink() { if ( _p_stream ) delete _p_stream; }

            std::string                         _name;

            std::ostream*                       _p_stream;

            unsigned int                        _logLevel;

            bool                                _stdstream;
        };

        std::vector< Sink* >                    _sinks;

        //! Stream buffer class
        class LogStreamBuf : public std::basic_streambuf< char >
        {
            public:

                                                LogStreamBuf();

                virtual                         ~LogStreamBuf();

                void                            setLog( Log *p_log );

            protected:

                virtual int_type                overflow( int_type c );

                Log*                            _p_log;

                std::string                     _msg;


        }                                       _stream;

};

// stream operator for setting log severity
std::ostream& operator << ( std::ostream& stream, Log::LogLevel& ll );

} // namespace CTD

#endif //_CTD_LOG_H_
