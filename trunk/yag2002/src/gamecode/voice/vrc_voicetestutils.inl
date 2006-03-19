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
 # utilities for testing the voice chat related stuff
 #
 #   date of creation:  03/18/2006
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_VOICETESTUTILS_H_
#error "do not include this file directly, include vrc_voicetestutils.h instead"
#endif

template< typename DataTypeT >
WaveWriter< DataTypeT >::WaveWriter( unsigned samplerate, unsigned int channelmode ) :
_rate( samplerate )
{
    assert( ( channelmode == WaveWriter::MONO ) || ( channelmode == WaveWriter::STEREO ) && "wrong channel mode!" );

    if ( channelmode == MONO )
        _channels = 1;
    else
        _channels = 2;

    _bits = sizeof( DataTypeT ) * 8;
}

template< typename DataTypeT >
WaveWriter< DataTypeT >::~WaveWriter()
{
}

template< typename DataTypeT >
void WaveWriter< DataTypeT >::writeHeader( std::fstream& stream, unsigned int size )
{
    typedef struct
    {
        char        _id[ 4 ];                   /* "RIFF" */
        int         _size;
        char        _formatTag[ 4 ];            /* format type  "WAVE" */
    } RiffChunk;

    typedef struct 
    {
        char            _subChunk1ID[ 4 ];      /* "fmt " */
        unsigned int    _subChunk1Size;         /* 16 for PCM  */
        unsigned short  _audioFormat;           /* 1 for PCM  */
        unsigned short  _channels;              /* number of channels (i.e. mono, stereo...)  */
        unsigned int    _samplesPerSec;         /* sample rate  */
        unsigned int    _avgBytesPerSec;        /* for buffer estimation  */
        unsigned short  _blockAlign;            /* block size of data  */
        unsigned short  _bitsPerSample;         /* number of bits per sample of mono data */
    } FmtChunk;

    typedef struct 
    {
        char            _subChunk2ID[ 4 ];      /* "data" */
        unsigned int    _subChunk2Size;
    } DataChunk;

    typedef struct
    {
        RiffChunk       _riffchunk;
        FmtChunk        _fmtchunk;
        DataChunk       _datachunk;
    } WavHeader;
    
    WavHeader header;
    // fill in the RIFF chunk
    strcpy( header._riffchunk._id, "RIFF" );
    header._riffchunk._size = 36 + _rate * _channels * _bits / 8;
    strcpy( header._riffchunk._formatTag, "WAVE" );
    // fill in the subchunk 1
    strcpy( header._fmtchunk._subChunk1ID, "fmt " );
    header._fmtchunk._subChunk1Size  = 16; // 16 for PCM
    header._fmtchunk._audioFormat    = 1;  // 1  for PCM
    header._fmtchunk._channels       = _channels;
    header._fmtchunk._samplesPerSec  = _rate;
    header._fmtchunk._avgBytesPerSec = _rate * _channels * _bits / 8;
    header._fmtchunk._blockAlign     = _channels * _bits / 8;
    header._fmtchunk._bitsPerSample  = _bits;
    // fill in the subchunk 2
    strcpy( header._datachunk._subChunk2ID, "data" );
    header._datachunk._subChunk2Size = size * _channels * _bits / 8;

    // write out the header
    char* p_buffer = reinterpret_cast< char* >( &header );
    unsigned int headersize = sizeof( header );
    for ( unsigned int cnt = 0; cnt < headersize; ++cnt )
        stream << p_buffer[ cnt ];
}

template< typename DataTypeT >
void WaveWriter< DataTypeT >::writeSample( std::fstream& stream, DataTypeT sample )
{
    // write out one single sample
    unsigned int mask     = 0xff;
    unsigned int datasize = sizeof( DataTypeT );
    for ( unsigned int bytes = 0; bytes < datasize; ++bytes )
    {
        char c = static_cast< char >( ( sample & mask ) >> ( 8 * bytes ) );
        stream << c;
        mask = mask << 8;
    }
}

template< typename DataTypeT >
bool WaveWriter< DataTypeT >::write( const std::string& filename, const std::vector< DataTypeT >& data )
{
    std::fstream file;
    file.open( filename.c_str(), std::ios_base::out | std::ios_base::binary );
    if ( !file )
        return false;

    writeHeader( file, data.size() );

    std::vector< DataTypeT >::const_iterator p_beg = data.begin(), p_end = data.end();
    for ( ; p_beg != p_end; ++p_beg )
        writeSample( file, *p_beg );

    file.close();

    return true;
}

template< typename DataTypeT >
bool WaveWriter< DataTypeT >::write( const std::string& filename, const std::queue< DataTypeT >& data )
{
    std::fstream file;
    file.open( filename.c_str(), std::ios_base::out | std::ios_base::binary );
    if ( !file )
        return false;

    writeHeader( file, data.size() );

    // write out the samples
    std::queue< DataTypeT > tmpqueue( data );
    while ( !tmpqueue.empty() )
    {
        writeSample( file, tmpqueue.front() );
        tmpqueue.pop();
    }

    file.close();

    return true;
}
