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
 # exception logger for writing a crash info file when an unhandled
 #  exception in application occurs.
 #
 # this code is basing on Matt Pietrek's article in 
 # Microsoft Systems Journal, April 1997 ( MSJEXHND.CPP / .H )
 #
 #   date of creation:  06/23/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/


#include "vrc_main.h"
#include <utils.h>
#include "vrc_exceptionlogger.h"

#ifdef WIN32

#pragma warning( disable : 4311 )
#pragma warning( disable : 4312 )

// Declare global instance of class
MSJExceptionHandler exceptionHandler;

TCHAR                        MSJExceptionHandler::_p_logFileName[ MAX_PATH ];
LPTOP_LEVEL_EXCEPTION_FILTER MSJExceptionHandler::_p_previousFilter;
HANDLE                       MSJExceptionHandler::_reportFile;

MSJExceptionHandler::MSJExceptionHandler( )
{
    // Install the unhandled exception filter function
    _p_previousFilter = SetUnhandledExceptionFilter( unhandledExceptionFilter );

    // Figure out what the report file will be named, and store it away
    GetModuleFileName( 0, _p_logFileName, MAX_PATH );

    // Look for the '.' before the "EXE" extension.  Replace the extension with "crashreport"
    PTSTR pszDot = _tcsrchr( _p_logFileName, _T('.') );
    if ( pszDot )
    {
        pszDot++;   // Advance past the '.'
        if ( _tcslen( pszDot ) >= 3 )
            _tcscpy( pszDot, _T( "crashreport" ) );
    }
}

MSJExceptionHandler::~MSJExceptionHandler( )
{
    SetUnhandledExceptionFilter( _p_previousFilter );
}

void MSJExceptionHandler::setLogFileName( PTSTR logFileName )
{
    _tcscpy( _p_logFileName, logFileName );
}

LONG WINAPI MSJExceptionHandler::unhandledExceptionFilter( PEXCEPTION_POINTERS p_exceptionInfo )
{
    _reportFile = CreateFile( _p_logFileName,
        GENERIC_WRITE,
        0,
        0,
        OPEN_ALWAYS,
        FILE_FLAG_WRITE_THROUGH,
        0 );

    if ( _reportFile )
    {
        SetFilePointer( _reportFile, 0, 0, FILE_END );

        generateExceptionReport( p_exceptionInfo );

        CloseHandle( _reportFile );
        _reportFile = 0;
    }

    if ( _p_previousFilter )
        return _p_previousFilter( p_exceptionInfo );
    else
        return EXCEPTION_CONTINUE_SEARCH;
}

void MSJExceptionHandler::generateExceptionReport( PEXCEPTION_POINTERS p_exceptionInfo )
{
    // Start out with a banner
    _tprintf( _T("//=====================================================\n") );

    PEXCEPTION_RECORD p_exceptionRecord = p_exceptionInfo->ExceptionRecord;

    // First print information about the type of fault
    _tprintf(   _T("Exception code: %08X %s\n"),
        p_exceptionRecord->ExceptionCode,
        getExceptionString(p_exceptionRecord->ExceptionCode) );

    // Now print information about where the fault occured
    TCHAR szFaultingModule[MAX_PATH];
    DWORD section, offset;
    getLogicalAddress(  p_exceptionRecord->ExceptionAddress,
        szFaultingModule,
        sizeof( szFaultingModule ),
        section, offset );

    _tprintf( _T("Fault address:  %08X %02X:%08X %s\n"),
        p_exceptionRecord->ExceptionAddress,
        section, offset, szFaultingModule );

    PCONTEXT pCtx = p_exceptionInfo->ContextRecord;

    // Show the registers
#ifdef _M_IX86  // Intel Only!
    _tprintf( _T("\nRegisters:\n") );

    _tprintf(_T("EAX:%08X\nEBX:%08X\nECX:%08X\nEDX:%08X\nESI:%08X\nEDI:%08X\n"),
        pCtx->Eax, pCtx->Ebx, pCtx->Ecx, pCtx->Edx, pCtx->Esi, pCtx->Edi );

    _tprintf( _T("CS:EIP:%04X:%08X\n"), pCtx->SegCs, pCtx->Eip );
    _tprintf( _T("SS:ESP:%04X:%08X  EBP:%08X\n"),
        pCtx->SegSs, pCtx->Esp, pCtx->Ebp );
    _tprintf( _T("DS:%04X  ES:%04X  FS:%04X  GS:%04X\n"),
        pCtx->SegDs, pCtx->SegEs, pCtx->SegFs, pCtx->SegGs );
    _tprintf( _T("Flags:%08X\n"), pCtx->EFlags );

    // Walk the stack using x86 specific code
    intelStackWalk( pCtx );

#endif

    _tprintf( _T("\n") );
}

LPTSTR MSJExceptionHandler::getExceptionString( DWORD code )
{
#define EXCEPTION( x ) case EXCEPTION_##x: return _T(#x);

    switch ( code )
    {   
        EXCEPTION( ACCESS_VIOLATION )
        EXCEPTION( DATATYPE_MISALIGNMENT )
        EXCEPTION( BREAKPOINT )
        EXCEPTION( SINGLE_STEP )
        EXCEPTION( ARRAY_BOUNDS_EXCEEDED )
        EXCEPTION( FLT_DENORMAL_OPERAND )
        EXCEPTION( FLT_DIVIDE_BY_ZERO )
        EXCEPTION( FLT_INEXACT_RESULT )
        EXCEPTION( FLT_INVALID_OPERATION )
        EXCEPTION( FLT_OVERFLOW )
        EXCEPTION( FLT_STACK_CHECK )
        EXCEPTION( FLT_UNDERFLOW )
        EXCEPTION( INT_DIVIDE_BY_ZERO )
        EXCEPTION( INT_OVERFLOW )
        EXCEPTION( PRIV_INSTRUCTION )
        EXCEPTION( IN_PAGE_ERROR )
        EXCEPTION( ILLEGAL_INSTRUCTION )
        EXCEPTION( NONCONTINUABLE_EXCEPTION )
        EXCEPTION( STACK_OVERFLOW )
        EXCEPTION( INVALID_DISPOSITION )
        EXCEPTION( GUARD_PAGE )
        EXCEPTION( INVALID_HANDLE )
    }

    // If not one of the "known" exceptions, try to get the string
    // from NTDLL.DLL's message table.
    static TCHAR p_buffer[ 512 ] = { 0 };

    FormatMessage(  
        FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
        GetModuleHandle( _T("NTDLL.DLL") ),
        code, 0, p_buffer, sizeof( p_buffer ), 0 
        );

    return p_buffer;
}

BOOL MSJExceptionHandler::getLogicalAddress( PVOID p_addr, PTSTR p_module, DWORD len, DWORD& section, DWORD& offset )
{
    MEMORY_BASIC_INFORMATION mbi;

    if ( !VirtualQuery( p_addr, &mbi, sizeof(mbi) ) )
        return FALSE;

    DWORD hMod = (DWORD)mbi.AllocationBase;

    if ( !GetModuleFileName( (HMODULE)hMod, p_module, len ) )
        return FALSE;

    // Point to the DOS header in memory
    PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;

    // From the DOS header, find the NT (PE) header
    PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);

    PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION( pNtHdr );

    DWORD rva = (DWORD)p_addr - hMod; // RVA is offset from module load address

    // Iterate through the section table, looking for the one that encompasses
    // the linear address.
    for (   unsigned i = 0; i < pNtHdr->FileHeader.NumberOfSections; i++, pSection++ )
    {
        DWORD sectionStart = pSection->VirtualAddress;
        DWORD sectionEnd = sectionStart
            + std::max(pSection->SizeOfRawData, pSection->Misc.VirtualSize);

        // Is the address in this section???
        if ( (rva >= sectionStart) && (rva <= sectionEnd) )
        {
            // Yes, address is in the section.  Calculate section and offset,
            // and store in the "section" & "offset" params, which were
            // passed by reference.
            section = i+1;
            offset = rva - sectionStart;
            return TRUE;
        }
    }

    return FALSE;   // Should never get here!
}

void MSJExceptionHandler::intelStackWalk( PCONTEXT p_context )
{
    _tprintf( _T("\nCall stack:\n") );

    _tprintf( _T("Address   Frame     Logical addr  Module\n") );

    DWORD pc = p_context->Eip;
    PDWORD p_frame, p_prevFrame;

    p_frame = (PDWORD)p_context->Ebp;

    do
    {
        TCHAR p_module[ MAX_PATH ] = _T("");
        DWORD section = 0, offset = 0;

        getLogicalAddress( ( PVOID )pc, p_module, sizeof( p_module ), section, offset );

        _tprintf( _T("%08X  %08X  %04X:%08X %s\n"),
            pc, p_frame, section, offset, p_module );

        pc = p_frame[1];

        p_prevFrame = p_frame;

        p_frame = ( PDWORD )p_frame[ 0 ]; // precede to next higher frame on stack

        if ( (DWORD)p_frame & 3 )    // Frame pointer must be aligned on a
            break;                  // DWORD boundary.  Bail if not so.

        if ( p_frame <= p_prevFrame )
            break;

        // Can two DWORDs be read from the supposed frame address?          
        if ( IsBadWritePtr( p_frame, sizeof( PVOID ) * 2 ) )
            break;

    } while ( 1 );
}

int __cdecl MSJExceptionHandler::_tprintf( const TCHAR * format, ... )
{
    TCHAR   p_buffer[ 1024 ];
    int     retValue;
    DWORD   cbWritten;
    va_list argptr;

    va_start( argptr, format );
    retValue = wvsprintf( p_buffer, format, argptr );
    va_end( argptr );

    WriteFile( _reportFile, p_buffer, retValue * sizeof( TCHAR ), &cbWritten, 0 );

    return retValue;
}

#endif // WIN32
