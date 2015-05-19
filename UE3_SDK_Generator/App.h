#ifndef __APP__
#define __APP__

#include <windows.h>
#include <time.h>
#include <fstream>
#include <Psapi.h>

using namespace std;

#pragma comment( lib, "Psapi.lib" )

class CAppTools
{
public:
	char*		GetDirectoryFileA( char *szFile );
	char*		GetDirectoryFileFormatA( char *pszFormat, ... );
	wchar_t*	GetDirectoryFileW( wchar_t *szFile );
	wchar_t*	GetDirectoryFileFormatW( wchar_t *pszFormat, ... );
	void		AddToLogFileA( char *szFile, char *szLog, ... );
	void		AddToLogFileW( wchar_t *szFile, wchar_t *szLog, ... );
	void		BaseUponModule( HMODULE hModule );
	MODULEINFO	GetSelfModuleInformation(){ return m_hMISelf; }
	bool		IsAddressInBaseImage( DWORD Address );
	
	HMODULE		m_hSelf;
	MODULEINFO	m_hMISelf;
	ofstream	ofile;	
	char		dlldir[ 320 ];
};

extern CAppTools GApp;

#endif