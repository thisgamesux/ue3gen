#include "stdafx.h"
#include <windows.h>
#include "App.h"

CAppTools GApp;

char* CAppTools::GetDirectoryFileA( char *szFile )
{
	static char path[ 320 ];
	strcpy( path, dlldir );
	strcat( path, szFile );
	return path;
}

char* CAppTools::GetDirectoryFileFormatA( char *pszFormat, ... )
{
	char Finalized[ MAX_PATH ] = { 0 };

	va_list va_alist;

	va_start( va_alist, pszFormat );

	_vsnprintf( Finalized + strlen( Finalized ), sizeof( Finalized ) - strlen( Finalized ), pszFormat, va_alist );

	va_end( va_alist );

	return GetDirectoryFileA( Finalized );
}

wchar_t* CAppTools::GetDirectoryFileW( wchar_t *szFile )
{
	char szFilename[ MAX_PATH ] = { 0 };

	wcstombs( szFilename, szFile, MAX_PATH );

	char *szDirectoryFile = GetDirectoryFileA( szFilename );

	static wchar_t szDirectoryFileW[ MAX_PATH ] = { 0 };

	mbstowcs( szDirectoryFileW, szDirectoryFile, MAX_PATH );

	return szDirectoryFileW;
}

wchar_t* CAppTools::GetDirectoryFileFormatW( wchar_t *pszFormat, ... )
{
	wchar_t Finalized[ MAX_PATH ] = { 0 };

	va_list va_alist;

	va_start( va_alist, pszFormat );

	_vsnwprintf( Finalized + wcslen( Finalized ), sizeof( Finalized ) - wcslen( Finalized ), pszFormat, va_alist );

	va_end( va_alist );

	return GetDirectoryFileW( Finalized );
}

void CAppTools::AddToLogFileA( char *szFile, char *szLog, ... )
{
	va_list va_alist;
	
	char logbuf[ 1024 ] = { 0 };
	
	FILE * fp;

	struct tm * current_tm;

	time_t current_time;

	time( &current_time );
	current_tm = localtime( &current_time );

	sprintf( logbuf, "[ %02d:%02d:%02d ] ", current_tm->tm_hour, current_tm->tm_min, current_tm->tm_sec );

	va_start( va_alist, szLog );
	
	_vsnprintf( logbuf + strlen( logbuf ), 
		sizeof( logbuf ) - strlen( logbuf ), 
		szLog, va_alist);

	va_end( va_alist );

	if ( ( fp = fopen ( GetDirectoryFileA( szFile ), "a" ) ) != NULL )
	{
		fprintf( fp, "%s\n", logbuf );
		fclose( fp );
	}
}

void CAppTools::AddToLogFileW( wchar_t *szFile, wchar_t *szLog, ... )
{
	va_list va_alist;

	wchar_t logbuf[ 1024 ] = { 0 };
	
	FILE * fp;
	
	struct tm * current_tm;
	
	time_t current_time;

	time (&current_time);
	
	current_tm = localtime (&current_time);

	wsprintf( logbuf, L"[ %02d:%02d:%02d ] ", current_tm->tm_hour, current_tm->tm_min, current_tm->tm_sec );

	va_start( va_alist, szLog );

	_vsnwprintf( logbuf + wcslen( logbuf ), 
		sizeof( logbuf ) - wcslen( logbuf ), 
		szLog, va_alist);
	
	va_end( va_alist );

	if ( ( fp = _wfopen( GetDirectoryFileW( szFile ), L"a" ) ) != NULL )
	{
		fwprintf( fp, L"%s\n", logbuf );
		fclose( fp );
	}
}

void CAppTools::BaseUponModule( HMODULE hModule )
{
	m_hSelf = hModule;

	GetModuleInformation( GetCurrentProcess(), m_hSelf, &m_hMISelf, sizeof( m_hMISelf ) );

  	GetModuleFileNameA( hModule, dlldir, 512 );

	for(int i = ( int )strlen(dlldir); i > 0; i--) 
	{ 
		if(dlldir[i] == '\\') 
		{ 
			dlldir[i+1] = 0; break; 
		} 
	}
}

bool CAppTools::IsAddressInBaseImage( DWORD Address )
{
	DWORD Begin = ( DWORD ) GetSelfModuleInformation().lpBaseOfDll;
	DWORD Endin = Begin + GetSelfModuleInformation().SizeOfImage;

	return ( Address > Begin && Address < Endin );
}