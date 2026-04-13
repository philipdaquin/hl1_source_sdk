#define __INTERFACE_H__
#include <tier1/interface.h>
#include "vstdlib/IKeyValuesSystem.h"
#include "KeyValues.h"
#include "KeyValuesCompat.h"
#include <cstdio>
#include <deque>
#include <string>
#include <unordered_map>
#include <cstdlib>

//This is the VGUI2 version of the IKeyValuesSystem interface. It has additional methods that makes it incompatible. - Solokiller
class IKeyValues : public IBaseInterface
{
public:
	// registers the size of the KeyValues in the specified instance
	// so it can build a properly sized memory pool for the KeyValues objects
	// the sizes will usually never differ but this is for versioning safety
	virtual void RegisterSizeofKeyValues( int size ) = 0;

	// allocates/frees a KeyValues object from the shared mempool
	virtual void *AllocKeyValuesMemory( int size ) = 0;
	virtual void FreeKeyValuesMemory( void *pMem ) = 0;

	// symbol table access (used for key names)
	virtual HKeySymbol GetSymbolForString( const char *name ) = 0;
	virtual const char *GetStringForSymbol( HKeySymbol symbol ) = 0;

	//Used by GoldSource only. - Solokiller
	virtual void GetLocalizedFromANSI( const char* ansi, wchar_t* outBuf, int unicodeBufferSizeInBytes ) = 0;

	virtual void GetANSIFromLocalized( const wchar_t* wchar, char* outBuf, int ansiBufferSizeInBytes ) = 0;

	// for debugging, adds KeyValues record into global list so we can track memory leaks
	virtual void AddKeyValuesToMemoryLeakList( void *pMem, HKeySymbol name ) = 0;
	virtual void RemoveKeyValuesFromMemoryLeakList( void *pMem ) = 0;
};

#define IKEYVALUES_INTERFACE_VERSION "KeyValues003"

IKeyValues* g_pKeyValuesInterface = NULL;
static IKeyValuesSystem* g_pActiveKeyValuesSystem = NULL;

class CKeyValuesWrapper : public IKeyValuesSystem
{
public:
	void RegisterSizeofKeyValues( int size ) override
	{
		return g_pKeyValuesInterface->RegisterSizeofKeyValues( size );
	}

	void *AllocKeyValuesMemory( int size ) override
	{
		return g_pKeyValuesInterface->AllocKeyValuesMemory( size );
	}

	void FreeKeyValuesMemory( void *pMem ) override
	{
		g_pKeyValuesInterface->FreeKeyValuesMemory( pMem );
	}

	HKeySymbol GetSymbolForString( const char *name ) override
	{
		return g_pKeyValuesInterface->GetSymbolForString( name );
	}

	const char *GetStringForSymbol( HKeySymbol symbol ) override
	{
		return g_pKeyValuesInterface->GetStringForSymbol( symbol );
	}

	void AddKeyValuesToMemoryLeakList( void *pMem, HKeySymbol name ) override
	{
		g_pKeyValuesInterface->AddKeyValuesToMemoryLeakList( pMem, name );
	}

	void RemoveKeyValuesFromMemoryLeakList( void *pMem ) override
	{
		g_pKeyValuesInterface->RemoveKeyValuesFromMemoryLeakList( pMem );
	}
};

CKeyValuesWrapper g_KeyValuesSystem;

#ifdef __EMSCRIPTEN__
class CLocalKeyValuesSystem : public IKeyValuesSystem
{
public:
	void RegisterSizeofKeyValues( int size ) override
	{
		m_keyValueSize = size;
	}

	void *AllocKeyValuesMemory( int size ) override
	{
		if( size <= 0 )
			size = m_keyValueSize > 0 ? m_keyValueSize : static_cast<int>( sizeof( KeyValues ) );

		return std::malloc( static_cast<size_t>( size ) );
	}

	void FreeKeyValuesMemory( void *pMem ) override
	{
		std::free( pMem );
	}

	HKeySymbol GetSymbolForString( const char *name ) override
	{
		const std::string key = name ? name : "";
		const auto it = m_symbolLookup.find( key );
		if( it != m_symbolLookup.end() )
			return it->second;

		m_symbols.emplace_back( key );
		const HKeySymbol symbol = static_cast<HKeySymbol>( m_symbols.size() );
		m_symbolLookup.emplace( m_symbols.back(), symbol );
		return symbol;
	}

	const char *GetStringForSymbol( HKeySymbol symbol ) override
	{
		if( symbol <= 0 || static_cast<size_t>( symbol ) > m_symbols.size() )
			return "";

		return m_symbols[symbol - 1].c_str();
	}

	void AddKeyValuesToMemoryLeakList( void *pMem, HKeySymbol name ) override
	{
		(void)pMem;
		(void)name;
	}

	void RemoveKeyValuesFromMemoryLeakList( void *pMem ) override
	{
		(void)pMem;
	}

private:
	int m_keyValueSize = 0;
	std::deque<std::string> m_symbols;
	std::unordered_map<std::string, HKeySymbol> m_symbolLookup;
};

static CLocalKeyValuesSystem g_LocalKeyValuesSystem;
#endif

IKeyValuesSystem *keyvalues()
{
	return g_pActiveKeyValuesSystem ? g_pActiveKeyValuesSystem : &g_KeyValuesSystem;
}

bool KV_InitKeyValuesSystem( CreateInterfaceFn* pFactories, int iNumFactories )
{
#ifdef __EMSCRIPTEN__
	g_pActiveKeyValuesSystem = &g_LocalKeyValuesSystem;
	g_pActiveKeyValuesSystem->RegisterSizeofKeyValues( sizeof( KeyValues ) );
	std::printf( "[KV-COMPAT] __EMSCRIPTEN__ using local KeyValues allocator/system size=%zu\n", sizeof( KeyValues ) );
	return true;
#else
	for (int i = 0; i < iNumFactories; ++i)
	{
		if (!g_pKeyValuesInterface)
		{
			g_pKeyValuesInterface = (IKeyValues *)pFactories[i](IKEYVALUES_INTERFACE_VERSION, NULL);
		}
	}

	if( !g_pKeyValuesInterface )
		return false;

	g_pKeyValuesInterface->RegisterSizeofKeyValues( sizeof( KeyValues ) );
	g_pActiveKeyValuesSystem = &g_KeyValuesSystem;

	return true;
#endif
}
