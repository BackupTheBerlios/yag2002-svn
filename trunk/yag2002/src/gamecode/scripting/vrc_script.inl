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
 # class providing base mechanisms for loading and executing scripts
 #  currently Lua ( http://www.lua.org ) is supported as scripting
 #  language.
 #
 #   date of creation:  04/15/2006
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#ifndef _VRC_SCRIPT_H_
    #error "do not include this file directly, include vrc_script.h instead"
#endif

//! Variable accessible in loaded scripts which contains the script path
//! This variable can be used for loading other scripts e.g. via 'dofile'
#define SCRIPT_PATH_VAR "scriptpath"

// Template definitions of Param
template< typename TypeT >
BaseParam* Param< TypeT >::clone()
{
    Param< TypeT >* p_param = new Param< TypeT >( _value );
    return p_param;
}

// Template definitions of Params
template< typename TypeT >
void Params::add( TypeT param )
{
    Param< TypeT >* p_param = new Param< TypeT >( param );
    std::vector< BaseParam* >::push_back( p_param );
}

template< typename TypeT >
const TypeT& Params::getValue( unsigned int index )
{
    Param< TypeT >* p_param = static_cast< Param< TypeT >* >( at( index ) );
    return p_param->getValue();
}

template< typename TypeT >
void Params::setValue( unsigned int index, TypeT value )
{
    Param< TypeT >* p_param = static_cast< Param< TypeT >* >( at( index ) );
    p_param->setValue( value );
}

const std::type_info& Params::getTypeInfo( unsigned int index )
{
    return at( index )->getTypeInfo();
}

const std::size_t Params::size() const
{
    return std::vector< BaseParam* >::size();
}

unsigned int Params::getNumPassedParameters() const
{
    return _numParams;
}

// Template definitions of BaseScript
template< class T >
BaseScript< T >::BaseScript() :
_p_state( NULL ),
_valid( false ),
_methodTableIndex( 0 )
{
}

template< class T >
BaseScript< T >::~BaseScript()
{
    closeScript();
}

template< class T >
void BaseScript< T >::setupLuaLibs( lua_State* p_state, unsigned int usedlibs )
{
    luaopen_base( p_state );

//! TODO: fix the problem with luaopen_package on linux
#ifndef LINUX
    if ( usedlibs & BaseScript::LOADLIB )
    {
        luaopen_package( p_state );
        lua_settop( p_state, 0 );
    }
#endif
    if ( usedlibs & BaseScript::STRING )
    {
        luaopen_string( p_state );
        lua_settop( p_state, 0 );
    }
    if ( usedlibs & BaseScript::TABLE )
    {
        luaopen_table( p_state );
        lua_settop( p_state, 0 );
    }
    if ( usedlibs & BaseScript::DBG )
    {
        luaopen_debug( p_state );
        lua_settop( p_state, 0 );
    }
    if ( usedlibs & BaseScript::MATH )
    {
        luaopen_math( p_state );
        lua_settop( p_state, 0 );
    }
    if ( usedlibs & BaseScript::IO )
    {
        luaopen_io( p_state );
        lua_settop( p_state, 0 );
    }
}

template< class T >
void BaseScript< T >::loadScript( const std::string& luaModuleName, const std::string& scriptfile, unsigned int usedlibs ) throw ( ScriptingException )
{
    _scriptFile = scriptfile;

    std::string file( yaf3d::Application::get()->getMediaPath() + scriptfile );

    assert( ( _p_state == NULL ) && "script file already created!" );

    _p_state = lua_open();

    setupLuaLibs( _p_state, usedlibs );

    int status = luaL_loadfile( _p_state, file.c_str() );
    std::string errormsg;
    if ( status != 0 )
    {
        errormsg = lua_tostring ( _p_state, -1 );
        closeScript();
    }

    //! TODO: gather some useful information such as line number when a syntax error occurs
    if ( status == LUA_ERRFILE )
        throw ScriptingException( std::string( "BaseScript: cannot open Lua file: " ) + scriptfile + "\n  reason: " + errormsg );
    else if ( status == LUA_ERRSYNTAX )
        throw ScriptingException( std::string( "BaseScript: syntax error in Lua file: " ) + scriptfile + "\n  reason: " + errormsg );
    else if ( status != 0 )
        throw ScriptingException( std::string( "BaseScript: error loading Lua file: " ) + scriptfile + "\n  reason: " + errormsg );

    lua_newtable( _p_state );
    _methodTableIndex = lua_gettop( _p_state );

    // make the script path accessible to script so it can include other scripts using this vaiable 'scriptpath'
    std::string scriptpath( yaf3d::extractPath( file ) );
    scriptpath += "/";
    lua_pushstring( _p_state, SCRIPT_PATH_VAR );
    lua_pushlstring( _p_state, scriptpath.c_str(), scriptpath.length() );
    lua_settable( _p_state, LUA_GLOBALSINDEX );

    lua_pushstring( _p_state, luaModuleName.c_str() );
    lua_pushvalue( _p_state, _methodTableIndex );
    lua_settable( _p_state, LUA_GLOBALSINDEX );

    _valid = true;
}

template< class T >
void BaseScript< T >::closeScript()
{
    // destroy lua state
    if ( _p_state )
    {
        lua_close( _p_state );
        _p_state = NULL;
    }

    // cleanup the method entries
    typename std::vector< MethodDescriptor* >::iterator p_beg = _methods.begin(), p_end = _methods.end();
    for ( ; p_beg != p_end; ++p_beg )
        delete ( *p_beg );

    _methods.clear();

    _valid = false;
}

template< class T >
const std::string& BaseScript< T >::getScriptFileName()
{
    return _scriptFile;
}

template< class T >
void BaseScript< T >::printParamsInfo( const Params& params )
{
    std::stringstream msg;
    Params::const_iterator p_beg = params.begin(), p_end = params.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        if ( ( *p_beg )->getTypeInfo() == typeid( float ) )
            msg << "float ";
        else if ( ( *p_beg )->getTypeInfo() == typeid( int ) )
            msg << "int ";
        else if ( ( *p_beg )->getTypeInfo() == typeid( unsigned int ) )
            msg << "unsigned int ";
        else if ( ( *p_beg )->getTypeInfo() == typeid( double ) )
            msg << "double ";
        else if ( ( *p_beg )->getTypeInfo() == typeid( std::string ) )
            msg << "string ";
        else
            assert( NULL && "BaseScript< T >::printParamsInfo, unsupported parameter type" );
    }

    log_debug << "( " << msg.str() << ")";
}

template< class T >
int BaseScript< T >::exposedMethodProxy( lua_State* p_state )
{
    // get number of arguments passed from script on function call
    std::size_t nargs = lua_gettop( p_state ) + 1;

    int res = lua_islightuserdata( p_state, lua_upvalueindex( 1 ) );
    assert( res && "missing user data" );
    MethodDescriptor* p_entry = static_cast< MethodDescriptor* >( lua_touserdata( p_state, lua_upvalueindex( 1 ) ) );
    assert( p_entry && "could not find method in user data" );

    // get the instance
    T*      p_instance   = p_entry->_p_instance;
    Params& arguments    = *p_entry->_p_arguments;
    Params& returnvalues = *p_entry->_p_returnValues;

    // grab the arguments
    {
        if ( nargs > ( arguments.size() + 1 ) )
        {
            log_error << "script error: exposed method called with greater parameter count as defined, file: "
                      << p_instance->getScriptFileName()
                      << ", method name: "
                      << p_entry->_methodName
                      << std::endl;

            p_instance->printParamsInfo( arguments );
            log_error << std::endl;
            return 0;
        }
        for ( std::size_t cnt = 1; cnt < nargs; ++cnt )
        {
            // get the param type
            const std::type_info& typeinfo = arguments.getTypeInfo( cnt - 1 );

            if ( lua_isnumber( p_state, cnt ) )
            {
                if ( typeinfo == typeid( float ) )
                    arguments.template setValue< float >( cnt - 1, static_cast< float >( lua_tonumber( p_state, cnt ) ) );
                else if ( typeinfo == typeid( int ) )
                    arguments.template setValue< int >( cnt - 1, static_cast< int >( lua_tonumber( p_state, cnt ) ) );
                else if ( typeinfo == typeid( unsigned int ) )
                    arguments.template setValue< unsigned int >( cnt - 1, static_cast< unsigned int >( lua_tonumber( p_state, cnt ) ) );
                else if ( typeinfo == typeid( double ) )
                    arguments.template setValue< double >( cnt - 1, lua_tonumber( p_state, cnt ) );
                else if ( typeinfo == typeid( std::string ) )
                {
                    std::stringstream valstr;
                    valstr << lua_tonumber( p_state, cnt );
                    arguments.template setValue< std::string >( cnt - 1, valstr.str() );
                }
                else
                {
                    log_error << "script error: exposed method called with unsupported or wrong parameter type, file: " << p_instance->getScriptFileName() << ", method name: " << p_entry->_methodName;
                    p_instance->printParamsInfo( arguments );
                    log_error << std::endl;
                }
            }
            else if ( lua_isstring( p_state, cnt ) )
            {
                if ( typeinfo == typeid( std::string ) )
                    arguments.template setValue< std::string >( cnt - 1, lua_tostring( p_state, cnt ) );
                else
                {
                    log_error << "script error: exposed method called with unsupported or wrong parameter type, file: " << p_instance->getScriptFileName() << ", method name: " << p_entry->_methodName;
                    p_instance->printParamsInfo( arguments );
                    log_error << std::endl;
                }
            }
            else if ( lua_isboolean( p_state, cnt ) )
            {
                if ( typeinfo == typeid( bool ) )
                    arguments.template setValue< bool >( cnt - 1, lua_toboolean( p_state, cnt ) ? true : false );
                else
                {
                    log_error << "script error: exposed method called with unsupported or wrong parameter type, file: " << p_instance->getScriptFileName() << ", method name: " << p_entry->_methodName;
                    p_instance->printParamsInfo( arguments );
                    log_error << std::endl;
                }
            }
        }
    }

    // call method
    arguments._numParams = nargs;
    ( p_instance->*( p_entry->_ptrMethod ) )( arguments, returnvalues );

    // push return values onto Lua stack
    {
        Params::iterator p_beg = returnvalues.begin(), p_end = returnvalues.end();
        for ( unsigned int index = 0; p_beg != p_end; ++p_beg, ++index )
        {
            const std::type_info& tinfo = ( *p_beg )->getTypeInfo();

            if ( tinfo == typeid( float ) )
                lua_pushnumber( p_instance->_p_state, GET_SCRIPT_PARAMVALUE( returnvalues, index, float ) );
            else if ( tinfo == typeid( int ) )
                lua_pushnumber( p_instance->_p_state, GET_SCRIPT_PARAMVALUE( returnvalues, index, int ) );
            else if ( tinfo == typeid( unsigned int ) )
                lua_pushnumber( p_instance->_p_state, GET_SCRIPT_PARAMVALUE( returnvalues, index, unsigned int ) );
            else if ( tinfo == typeid( double ) )
                lua_pushnumber( p_instance->_p_state, GET_SCRIPT_PARAMVALUE( returnvalues, index, double ) );
            else if ( tinfo == typeid( std::string ) )
                lua_pushstring( p_instance->_p_state, ( GET_SCRIPT_PARAMVALUE( returnvalues, index, std::string ) ).c_str() );
            else if ( tinfo == typeid( bool ) )
                lua_pushboolean( p_instance->_p_state, ( GET_SCRIPT_PARAMVALUE( returnvalues, index, bool ) ) ? 1 : 0 );
        }
    }

    // return number of return values
    return returnvalues.size();
}

template< class T >
void BaseScript< T >::exposeMethod( const std::string& name, MethodPtr method, const Params& arguments, const Params& returnvalues )
{
    // create a method description
    MethodDescriptor* p_ms  = new MethodDescriptor;
    p_ms->_p_instance       = static_cast< T* >( this );
    p_ms->_methodName       = name;
    p_ms->_ptrMethod        = method;
    *p_ms->_p_arguments     = arguments;
    *p_ms->_p_returnValues  = returnvalues;
    _methods.push_back( p_ms );

    // register the exposed method in Lua
    lua_pushstring( _p_state, name.c_str() );
    lua_pushlightuserdata( _p_state, p_ms );
    lua_pushcclosure( _p_state, exposedMethodProxy, 1 );
    lua_settable( _p_state, _methodTableIndex );
}

template< class T >
void BaseScript< T >::callScriptFunction( const std::string& fcnname, Params* const p_arguments, Params* p_returnvalues ) throw ( ScriptingException )
{
    if ( !_valid )
    {
        log_error << "script error: cannot call function of invalid script: " << _scriptFile << std::endl;
        throw( ScriptingException( "BasicScript: invalid script, cannot call script function" ) );
    }

    // get number of return values
    int numret = p_returnvalues ? p_returnvalues->size() : 0;

    lua_getglobal( _p_state, fcnname.c_str() );

    // push arguments to Lua stack
    {
        Params::iterator p_beg = p_arguments->begin(), p_end = p_arguments->end();
        for ( unsigned int index = 0; p_beg != p_end; ++p_beg, ++index )
        {
            const std::type_info& tinfo = ( *p_beg )->getTypeInfo();

            if ( tinfo == typeid( float ) )
                lua_pushnumber( _p_state, GET_SCRIPT_PARAMVALUE( *p_arguments, index, float ) );
            else if ( tinfo == typeid( int ) )
                lua_pushnumber( _p_state, GET_SCRIPT_PARAMVALUE( *p_arguments, index, int ) );
            else if ( tinfo == typeid( unsigned int ) )
                lua_pushnumber( _p_state, GET_SCRIPT_PARAMVALUE( *p_arguments, index, unsigned int ) );
            else if ( tinfo == typeid( double ) )
                lua_pushnumber( _p_state, GET_SCRIPT_PARAMVALUE( *p_arguments, index, double ) );
            else if ( tinfo == typeid( std::string ) )
                lua_pushstring( _p_state, ( GET_SCRIPT_PARAMVALUE( *p_arguments, index, std::string ) ).c_str() );
        }
    }

    bool success = true;
    std::string scripterror;
    try
    {
        // call the Lua function
        lua_call( _p_state, p_arguments->size(), numret );
    }
    catch ( ... )
    {
        success = false;
        scripterror = lua_tostring( _p_state, -1 );
        log_error << "BaseScript: error executing script function : " << fcnname << ", reason: " << scripterror << std::endl;
    }

    // throw an scripting exception if the call was not successful
    if ( !success )
        throw( ScriptingException( scripterror ) );

    // get return values
    if ( numret > 0 )
    {
        int sindex = -numret;
        for ( int cnt = 0; cnt < numret; ++cnt, ++sindex )
        {
            // get the param type
            const std::type_info& typeinfo = p_returnvalues->getTypeInfo( cnt );
            if ( lua_isnumber( _p_state, sindex ) )
            {
                if ( typeinfo == typeid( float ) )
                    p_returnvalues->template setValue< float >( cnt, static_cast< float >( lua_tonumber( _p_state, sindex ) ) );
                else if ( typeinfo == typeid( int ) )
                    p_returnvalues->template setValue< int >( cnt, static_cast< int >( lua_tonumber( _p_state, sindex ) ) );
                else if ( typeinfo == typeid( unsigned int ) )
                    p_returnvalues->template setValue< unsigned int >( cnt, static_cast< unsigned int >( lua_tonumber( _p_state, sindex ) ) );
                else if ( typeinfo == typeid( double ) )
                    p_returnvalues->template setValue< double >( cnt, lua_tonumber( _p_state, sindex ) );
                else
                {
                    log_error << "script error: return value(s) mismatch occured during calling script function: " << fcnname << std::endl;
                }
            }
            else if ( lua_isstring( _p_state, sindex ) )
            {
                if ( typeinfo == typeid( std::string ) )
                    p_returnvalues->template setValue< std::string >( cnt, lua_tostring( _p_state, sindex ) );
                else
                {
                    log_error << "script error: return value(s) mismatch occured during calling script function: " << fcnname << std::endl;
                }
            }
            else
                log_error << "script error: cannot retrieve return value for script function: " << fcnname << std::endl;
        }
    }

    // restore Lua stack
    lua_pop( _p_state, static_cast< int >( numret ) );
}

template< class T >
void BaseScript< T >::execute() throw ( ScriptingException )
{
    if ( !_valid )
    {
        log_error << "script error: cannot execute invalid script: " << _scriptFile << std::endl;
        return;
    }

    bool success = true;
    std::string scripterror;

    try
    {
        lua_pop( _p_state, 1 );
        lua_call( _p_state, 0, 0 );
    }
    catch ( ... )
    {
        success = false;
        scripterror = lua_tostring( _p_state, -1 );
    }

    // throw an scripting exception
    if ( !success )
        throw( ScriptingException( scripterror ) );
}
