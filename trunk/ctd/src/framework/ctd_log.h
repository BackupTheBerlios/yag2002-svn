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

#ifndef _CTD_LOG_H_
#define _CTD_LOG_H_

#include <ctd_base.h>

namespace CTD
{

class Log;

//! This is the default system log instance
/** Usage:
*            log << Log::LogLevel( Log::L_INFO ) << " this is an info log" << endl;
*            log << " number: " << mynum << " string: " << mystring << endl;
*/
extern Log log;

//! Class for logging messages to defined sinks
class Log : public std::basic_ostream< char >
{
    public:

        //! Logging thresholds
        enum Level {
            L_ERROR   = 0x1,
            L_WARNING = 0x2,
            L_DEBUG   = 0x3,
            L_INFO    = 0x4
        };

                                                    Log();

        virtual                                     ~Log();

        //! Add a file sink
		bool                                        addSink( const std::string& sinkname, const std::string& filename, unsigned int loglevel = Log::L_DEBUG );

        //! Add standard sink such as cout
		bool                                        addSink( const std::string& sinkname, std::ostream& sink = std::cout, unsigned int loglevel = Log::L_DEBUG );

        //! Remove a sink given its name
        void                                        removeSink( const std::string& sinkname );

        //! Output a message on sinks with given severity
        void                                        out( const std::string& msg );

        //! Set current message severity
        void                                        setSeverity( unsigned int severity );

        //! Enables/disables severity level printing in output, the default is 'enabled'.
        void                                        enableSeverityLevelPrinting( bool en );

        //! Struct for setting new loglevel via stream operator <<
        struct LogLevel
        {
                LogLevel( Level threshold ) : _level( threshold ) {}

                Level     _level;
        };

        //! Stream operator for setting current logging severity
        std::ostream&                               operator << ( const Log::LogLevel& ll );

    protected:

        //! Avoid assignment operator
        Log&                                        operator = ( const Log& log );

        //! Avoid copy constructor
                                                    Log( const Log& );

        //! Currently set severity
        unsigned int                                _severity;

        //! Severity level printing 
        bool                                        _printSeverityLevel;

        //! Log sink 
        class Sink
        {
            public:

                                                        Sink( const std::string& name, std::ostream* p_stream, unsigned int loglevel, bool stdstream = false ) :
                                                            _p_stream( p_stream ), 
                                                            _logLevel( loglevel ),
                                                            _name( name ),
                                                            _stdstream( stdstream )
                                                        {}
                                                        
            virtual                                     ~Sink() { if ( _p_stream && !_stdstream ) delete _p_stream; }

            std::string                                 _name;

            std::ostream*                               _p_stream;

            unsigned int                                _logLevel;

            bool                                        _stdstream;
        };

        std::vector< Sink* >                        _sinks;

        //! Stream buffer class
        class LogStreamBuf : public std::basic_streambuf< char >
        {
            public:

                                                        LogStreamBuf();

                virtual                                 ~LogStreamBuf();

                void                                    setLog( Log *p_log );

            protected:

                virtual int_type                        overflow( int_type c );

                Log*                                    _p_log;

                std::string                             _msg;


        }                                               _stream;

};

} // namespace CTD

#endif //_CTD_LOG_H_
