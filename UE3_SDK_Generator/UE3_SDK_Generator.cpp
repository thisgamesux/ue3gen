// UE3_SDK_Generator.cpp : Defines the entry point for the DLL application.
// Coded by s0beit of [www.thisgamesux.net][www.gamedeception.net][www.uc-forum.com]

#include "stdafx.h"

struct FPadding
{
	ULONG											Offset;
	ULONG											Size;
};

struct FFunctionInfo
{
	UClass*											Class;
	UFunction*										Function;
};

struct FPackage
{
	std::wstring									PackageName;
	std::vector< UConst* >							Constant;
	std::vector< UEnum* >							Enum;
	std::vector< UStruct* >							Struct;
	std::vector< FFunctionInfo >					Function;
	std::vector< UClass* >							Class;
};

typedef stdext::hash_map< std::wstring, FPackage >	FPackageMap;
FPackageMap											GPackages;

bool CompareFoundData( const BYTE* pData, const BYTE* bMask, const char* szMask )
{
    for( ; *szMask; ++szMask, ++pData, ++bMask )
	{
        if( *szMask == 'x' && *pData != *bMask ) 
		{
            return false;
		}
	}
    return ( *szMask ) == NULL;
}

DWORD FindPattern( DWORD dwAddress, DWORD dwLen, BYTE *bMask, char* szMask )
{
    for( DWORD i=0; i < dwLen; i++ )
	{
		if( CompareFoundData( ( BYTE* )( dwAddress + i ), bMask, szMask) )
		{
			return ( DWORD )( dwAddress + i );
		}
	}
    return 0;
}

PCHAR GetExecutableName()
{
	static char dir[ MAX_PATH ];

	if( strlen( dir ) == 0 )
	{
		char temp[ MAX_PATH ];

		GetModuleFileNameA( GetModuleHandle( NULL ), temp, MAX_PATH );

		size_t IndexOfFile = strlen( temp );

		for( ; temp[ IndexOfFile ] != '\\'; IndexOfFile-- );

		IndexOfFile++; // Slash

		strcpy( dir, temp + IndexOfFile );
	}

	return dir;
}

PCHAR GetMonthFromTime( int m )
{
	static PCHAR Month[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
	
	return Month[ m ];
}

struct tm* GetGenerationTime()
{
	time_t t_tm;

	time( &t_tm );

	return localtime( &t_tm );
}

BOOL IsValidChild( UObject* u )
{
	std::wstring Name = u->GetName();

	return ( Name.substr( 0, 2 ).compare( L"__" ) == 0 || Name.find( L"__Delegate" ) != std::wstring::npos );
}

BOOL PropertyToString( UProperty* Property, std::string *s )
{
	if( s == NULL ) return FALSE;

	std::wstring PropertyName = Property->GetFullName();

	if( wcsstr( PropertyName.c_str(), L"DelegateProperty" ) )
	{
		*s = "struct FScriptDelegate";

		return TRUE;
	}
	
	if( wcsstr( PropertyName.c_str(), L"PointerProperty" ) )
	{
		*s = "PVOID";

		return TRUE;
	}
	
	//

	if( Property->IsA( UByteProperty::StaticClass() ) )
	{
		*s = "UCHAR";

		return TRUE;
	}
	
	if( Property->IsA( UIntProperty::StaticClass() ) )
	{
		*s = "INT";

		return TRUE;
	}
	
	if( Property->IsA( UFloatProperty::StaticClass() ) )
	{
		*s = "FLOAT";

		return TRUE;
	}
	
	/*
	if( Property->IsA( UBoolProperty::StaticClass() ) ) //obsolete
	{
		*s = "bool";

		return TRUE;
	}
	*/
	
	if( Property->IsA( UStrProperty::StaticClass() ) )
	{
		*s = "FString";

		return TRUE;
	}
	
	if( Property->IsA( UNameProperty::StaticClass() ) )
	{
		*s = "FName";

		return TRUE;
	}
	
	if( Property->IsA( UObjectProperty::StaticClass() ) )
	{
		char b[ 4096 ];

		sprintf( b, "class %S*", ( ( UObjectProperty* ) Property )->PropertyClass->GetNameCPP().c_str() );

		*s = b;

		return TRUE;
	}
	
	if( Property->IsA( UClassProperty::StaticClass() ) )
	{
		char b[ 4096 ];

		sprintf( b, "class %S*", ( ( UClassProperty* ) Property )->MetaClass->GetNameCPP().c_str() );

		*s = b;

		return TRUE;
	}
	
	if( Property->IsA( UStructProperty::StaticClass() ) )
	{
		char b[ 4096 ];

		sprintf( b, "struct %S", ( ( UStructProperty* ) Property )->Struct->GetNameCPP().c_str() );

		*s = b;

		return TRUE;
	}
	
	if( Property->IsA( UArrayProperty::StaticClass() ) )
	{
		std::string Basic;

		if( PropertyToString( ( ( UArrayProperty* ) Property )->Inner, &Basic ) == FALSE )
		{
			return FALSE;
		}

		(*s).append( "TArray< " );
		(*s).append( Basic );
		(*s).append( " >" );

		return TRUE;
	}

	return FALSE;
}

BOOL GenerateConst( UConst* Const, FILE* File )
{
	if( Const == NULL || File == NULL ) return FALSE;

	fprintf( File, "#define UCONST_%S %S\n", Const->GetName().c_str(), Const->Value );

	return TRUE;
}

BOOL GenerateEnum( UEnum* Enum, FILE* File )
{
	if( Enum == NULL || File == NULL ) return FALSE;

	fprintf( File, "enum %S\n{\n", Enum->GetName().c_str() );

	for( int i = 0; i < Enum->Names.Count; ++i )
	{
		if( i == ( Enum->Names.Count - 1 ) )
		{
			fprintf( File, "\t%S = 0x%X\n", Enum->Names.Data[ i ].GetName(), i );
		}
		else
		{
			fprintf( File, "\t%S = 0x%X,\n", Enum->Names.Data[ i ].GetName(), i );
		}
	}

	fprintf( File, "};\n\n" );

	return TRUE;
}

BOOL GenerateStruct( UStruct* Struct, FILE* File )
{
	if( Struct == NULL || File == NULL ) return FALSE;

	fprintf( File, "struct %S\n{\n", Struct->GetNameCPP().c_str() );

	for ( UProperty* StructChild = ( UProperty* ) Struct->Children; StructChild; StructChild = ( UProperty* ) StructChild->Next )
	{
		std::string propname;

		if( PropertyToString( StructChild, &propname ) )
		{
			fprintf( File, "\t%s %S; // [0x%X] [0x%X]\n", 
				propname.c_str(), 
				StructChild->GetName().c_str(), 
				StructChild->PropertyOffset,
				StructChild->ElementSize );
		}
	}

	fprintf( File, "};\n\n" );

	return TRUE;
}

BOOL GenerateFunction( FFunctionInfo* Function, FILE* File )
{
	if( Function == NULL || File == NULL ) return FALSE;

	fprintf( File, "struct %S_", Function->Class->GetNameCPP().c_str() );

	if( Function->Function->FunctionFlags & 0x00000800 )
	{
		fprintf( File, "event" );
	}
	else
	{
		fprintf( File, "exec" );
	}

	fprintf( File, "%S_Params // [0x%X]\n{\n", Function->Function->GetName().c_str(), Function->Function->FunctionFlags );

	for ( UProperty* FunctionChild = ( UProperty* ) Function->Function->Children; FunctionChild; FunctionChild = ( UProperty* ) FunctionChild->Next )
	{
		std::string propname;

		if( PropertyToString( FunctionChild, &propname ) )
		{
			std::string ListOfParameterTypes;

			if( FunctionChild->PropertyFlags & CPF_OptionalParm )
				ListOfParameterTypes.append( "[CPF_OptionalParm]" );

			if( FunctionChild->PropertyFlags & CPF_Parm )
				ListOfParameterTypes.append( "[CPF_Parm]" );

			if( FunctionChild->PropertyFlags & CPF_OutParm )
				ListOfParameterTypes.append( "[CPF_OutParm]" );

			if( FunctionChild->PropertyFlags & CPF_ReturnParm )
				ListOfParameterTypes.append( "[CPF_ReturnParm]" );

			if( FunctionChild->PropertyFlags & CPF_CoerceParm )
				ListOfParameterTypes.append( "[CPF_CoerceParm]" );

			if( ListOfParameterTypes.length() == 0 )
			{
				fprintf( File, "\t%s %S; // [0x%X] [0x%X]\n", 
					propname.c_str(), 
					FunctionChild->GetName().c_str(),
					FunctionChild->PropertyOffset,
					FunctionChild->ElementSize );
			}
			else
			{
				fprintf( File, "\t%s %S; //[0x%X] [0x%X] %s\n", 
					propname.c_str(), 
					FunctionChild->GetName().c_str(), 
					FunctionChild->PropertyOffset,
					FunctionChild->ElementSize,
					ListOfParameterTypes.c_str() );
			}
		}
	}

	if( ( Function->Function->FunctionFlags & 0x400 ) )
	{
		fprintf( File, "}; // CPF_Native [0x%X][0x%X]\n\n", Function->Function->FunctionFlags, Function->Function->iNative );
	}
	else
	{
		fprintf( File, "}; // [0x%X][0x%X]\n\n", Function->Function->FunctionFlags, Function->Function->iNative );
	}

	return TRUE;
}

// anyway so current bugs, ordering, missing bytes from classes

BOOL GenerateObjectVTable( UClass* Class, FILE* File )
{
	PDWORD *pdwTable = ( PDWORD* ) Class;

	if( *pdwTable )
	{
		fprintf( File, "\npublic:\n" );

		for( size_t i = 0; ( *pdwTable )[ i ]; i++ )
		{
			DWORD AddressOfFunction = ( *pdwTable )[ i ];

			GApp.AddToLogFileA( "virtual.log", "Virtual [%S] -> [%04i][0x%04X][0x%X]", 
				Class->GetNameCPP().c_str(), i, i * 4, AddressOfFunction );

			// F7 87 ? 00 00 00 00 04 - test dword ptr [edi+?], 400h

			DWORD dwFound = FindPattern( AddressOfFunction, 0x200, ( BYTE* ) "\xF7\x87\xE8\x00\x00\x00\x00\x04", ( CHAR* ) "xx?xxxxx" );

			// 6A FF - push 0FFFFFFFFh

			DWORD dwSanity = FindPattern( AddressOfFunction, 0x20, ( BYTE* ) "\x6A\xFF", ( CHAR* ) "xx" );

			if( dwFound && dwSanity )
			{
				fprintf( File, "\tvirtual void ProcessEvent( class UFunction* F, PVOID Parameters, PVOID Result = NULL ) = 0; // Address: 0x%X Offset: 0x%04X [%04i]\n",
					AddressOfFunction, i * 4, i );
			}
			else
			{
				fprintf( File, "\tvirtual void Unknown_%04i( void ) = 0; // Address: 0x%X Offset: 0x%04X [%04i]\n",
					i, AddressOfFunction, i * 4, i );
			}
		}

		return TRUE;
	}

	return FALSE;
}

bool PropertySortLogic( UProperty* a, UProperty* b )
{
	if( b == NULL )
		return false;

	if( a == NULL )
		return true;

	if( a->PropertyOffset < b->PropertyOffset ) return true;

	if( a->IsA( UBoolProperty::StaticClass() ) && b->IsA( UBoolProperty::StaticClass() ) )
	{
		return ( ( UBoolProperty* ) a )->Order < ( ( UBoolProperty* ) b )->Order;
	}

	return false;
}

BOOL GeneratePadding( UClass* Class, std::vector< UProperty* > *Children )
{
	ULONG ClassStart	= ( Class->SuperField ) ? ( ( ( UClass* ) Class->SuperField )->PropertySize ) : 0;
	ULONG ClassEnd		= ClassStart + Class->PropertySize; //Maybe....

	GApp.AddToLogFileA( "dbg.log", "Padding ClassStart [0x%X] ClassEnd [0x%X]", ClassStart, ClassEnd );

	if( Class->PropertySize == 0 )
	{
		GApp.AddToLogFileA( "dbg.log", "Blank Class" );

		return TRUE; // Blank...
	}
	else if( Children->size() == 0 )
	{
		GApp.AddToLogFileA( "dbg.log", "No Properties" );

		UProperty *N = new UProperty;

		N->PropertyOffset	= ClassStart;
		N->ArrayDim			= ClassEnd - ClassStart;
		N->ElementSize		= 1;
		N->Name.Index		= -1;

		Children->push_back( N );

		return TRUE; // No legit properties...
	}

	GApp.AddToLogFileA( "dbg.log", "Enumeration Begin!" );

	for( size_t i = 0; i < Children->size(); i++ )
	{
		UINT PropertyOffset		= ( *Children )[ i ]->PropertyOffset;
		UINT PropertyTotalSize	= ( *Children )[ i ]->ArrayDim * ( *Children )[ i ]->ElementSize;
		UINT NextPropertyOffset = ( PropertyOffset + PropertyTotalSize );

		GApp.AddToLogFileA( "dbg.log", "Children [%i][0x%X][0x%X] -> [0x%X]", i, PropertyOffset, PropertyTotalSize, NextPropertyOffset );

		if( i == Children->size() )
		{
			if( NextPropertyOffset != ClassEnd )
			{
				UProperty *N = new UProperty;

				N->PropertyOffset	= NextPropertyOffset;
				N->ElementSize		= 1;
				N->ArrayDim			= ClassEnd - NextPropertyOffset;
				N->Name.Index		= -1;

				Children->push_back( N );
			}
		}
		else
		{
			if( i == 0 && PropertyOffset != ClassStart )
			{
				UProperty *N = new UProperty;

				N->PropertyOffset	= ClassStart;
				N->ElementSize		= 1;
				N->ArrayDim			= PropertyOffset - ClassStart;
				N->Name.Index		= -1;

				Children->push_back( N );
			}
			else
			{
				UProperty* NextProperty = ( *Children )[ i + 1 ];

				if( NextProperty->PropertyOffset != NextPropertyOffset )
				{
					UProperty *N = new UProperty;

					N->PropertyOffset	= NextPropertyOffset;
					N->ElementSize		= 1;
					N->ArrayDim			= NextProperty->PropertyOffset - NextPropertyOffset;
					N->Name.Index		= -1;

					Children->push_back( N );
				}
			}
		}
	}

	return TRUE;
}

BOOL GenerateClass( UClass* Class, FILE* File )
{
	if( Class == NULL || File == NULL ) return FALSE;

	if( Class->PropertySize == 0 )
	{
		GApp.AddToLogFileA( "packages.log", "Empty class [%S]", Class->GetFullName().c_str() );

		return TRUE;
	}

	// Class header

	fprintf( File, "class %S", Class->GetNameCPP().c_str() );

	if( Class->SuperField && Class->SuperField != Class )
	{
		fprintf( File, " : public %S", Class->SuperField->GetNameCPP().c_str() );
	}

	fprintf( File, " // [0x%X][0x%X][0x%X]", Class->PropertySize, Class->ScriptText, Class->CppText );

	fprintf( File, "\n{\n" );

	// Children

	std::vector< UProperty* > Children;

	for( UProperty* Child = ( UProperty* ) Class->Children; Child; Child = ( UProperty* ) Child->Next )
	{
		Children.push_back( Child );
	}

	// Sort the first time so they are in order for the padding

	std::sort( Children.begin(), Children.end(), PropertySortLogic );

	GApp.AddToLogFileA( "dbg.log", "Collected children from class [%S]", Class->GetNameCPP().c_str() );

	GeneratePadding( Class, &Children );

	GApp.AddToLogFileA( "dbg.log", "Collected padding from class [%S]", Class->GetNameCPP().c_str() );

	// Sort the second time so they are in order for the enumeration

	std::sort( Children.begin(), Children.end(), PropertySortLogic );

	for( size_t i = 0, padding = 0; i < Children.size(); ++i )
	{
		UProperty* Prop = Children[ i ];

		if( Prop == NULL )
		{
			// What lol

			continue;
		}

		UINT PropertySize = ( Prop->ArrayDim * Prop->ElementSize );

		if( Prop->Name.Index == -1 )
		{
			// This is a generated padding member

			fprintf( File, "UCHAR %S_Padding_%04i[ 0x%X ]; // [0x%X]\n", Class->GetName().c_str(), padding, PropertySize, Prop->PropertyOffset );

			continue;
		}

		// We will have trouble down the line interpretting the virtual table by the adjusters below...

		if( ( wcscmp( Class->GetNameCPP().c_str(), L"UObject" ) == 0 ) &&
			( wcsncmp( Prop->GetName().c_str(), L"VfTable", 7 ) == 0 ) &&
			Prop->IsA( UStructProperty::StaticClass() ) )
		{
			GApp.AddToLogFileA( "table.log", "Found Virtual Table [%S][%S]", Class->GetNameCPP().c_str(), Prop->GetName().c_str() );

			GenerateObjectVTable( Class, File );

			fprintf( File, "\npublic:\n" );

			continue;
		}

		if( Prop->IsA( UBoolProperty::StaticClass() ) )
		{
			fprintf( File, "\tULONG %S : 1; // [0x%X][0x%X]\n", Prop->GetName().c_str(), Prop->PropertyOffset, PropertySize );
		}
		else
		{
			std::string propname;

			if( PropertyToString( Prop, &propname ) )
			{
				fprintf( File, "\t%s %S; // [0x%X][0x%X]\n", 
					propname.c_str(), 
					Prop->GetName().c_str(), 
					Prop->PropertyOffset,
					PropertySize );
			}
		}
	}

	fprintf( File,
		"\npublic:\n"
		"\tstatic UClass* Singleton()\n"
		"\t{\n"
		"\t\tstatic UClass* Static = NULL;\n\n"
		"\t\tif( Static == NULL ) Static = ( UClass* ) UObject::FindObject( \"%S\" );\n\n"
		"\t\treturn Static;\n"
		"\t};\n\n", Class->GetFullName().c_str() );

	if( wcscmp( Class->GetNameCPP().c_str(), L"UObject" ) == 0 )
	{
		fprintf( File,
			"\npublic:\n"
			"\tstatic TArray< UObject* >* Global();\n\n"
			"\tstd::wstring GetName();\n"
			"\tstd::wstring GetNameCPP();\n"
			"\tstd::wstring GetFullName();\n\n"
			"\tinline bool IsA( UClass* Class );\n\n"
			"\tstatic UObject* FindObject ( wchar_t* pObjectName );\n"
			"\tstatic UObject* FindObject ( char* pObjectName );\n" );
	}

	fprintf( File, "};\n\n" );

	return TRUE;
}

BOOL GenerateHeader( std::wstring PackageName, FILE* File )
{
	if( File == NULL ) return FALSE;

	struct tm* Time = GetGenerationTime();

	fprintf( File, 
		"/////////////////////////////////////////////\n"
		"// Unreal Engine 3 Generator by s0beit\n"
		"// Game: %s\n"
		"// File: %S.h\n"
		"// Date: %s/%i/%i\n"
		"/////////////////////////////////////////////\n\n"
		"#ifdef _MSC_VER\n"
		"#pragma pack( push, 4 )\n"
		"#endif\n\n",
		GetExecutableName(),
		PackageName.c_str(),
		GetMonthFromTime( Time->tm_mon ),
		Time->tm_mday,
		Time->tm_year + 1900 );

	return TRUE;
}

BOOL GenerateFooter( FILE* File )
{
	if( File == NULL ) return FALSE;

	fprintf( File, "#ifdef _MSC_VER\n#pragma pack( pop )\n#endif" );

	return TRUE;
}

VOID GenerateHeaders()
{
	if( UObject::GObjObjects() == NULL )
	{
		GApp.AddToLogFileA( "error.log", "GObjObjects NULL" );

		return;
	}

	// Store prototype files

	GApp.AddToLogFileA( "SDK.log", "Gathering prototype files.." );

	std::vector< WIN32_FIND_DATAA > Prototype;

	WIN32_FIND_DATAA Proto;

	HANDLE hProto = FindFirstFileA( GApp.GetDirectoryFileA( "Prototype\\*" ), &Proto );

	if( hProto != INVALID_HANDLE_VALUE )
	{
		while( true )
		{
			if( Proto.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				if( FindNextFileA( hProto, &Proto ) == FALSE )
				{
					break;
				}

				continue;
			}

			if( Proto.cFileName[ 0 ] == '.' )
			{
				if( FindNextFileA( hProto, &Proto ) == FALSE )
				{
					break;
				}

				continue;
			}

			GApp.AddToLogFileA( "SDK.log", "Prototype file [%s] being included", Proto.cFileName );

			Prototype.push_back( Proto );

			if( FindNextFileA( hProto, &Proto ) == FALSE )
			{
				break;
			}
		}
	}

	FindClose( hProto );

	// Generate headers individually

	GApp.AddToLogFileA( "SDK.log", "Collecting package data.." );

	for( int i = 0; i < UObject::GObjObjects()->Count; i++ )
	{
		UObject* Object = UObject::GObjObjects()->at( i );

		if( Object == NULL )
			continue;

		std::wstring PackageName = Object->GetPackageName();

		if( Object->IsA( UConst::StaticClass() ) )
		{
			GPackages[ PackageName ].Constant.push_back( ( UConst* ) Object );
		}
		else if( Object->IsA( UEnum::StaticClass() ) )
		{
			GPackages[ PackageName ].Enum.push_back( ( UEnum* ) Object );
		}
		else if( Object->IsA( UClass::StaticClass() ) )
		{
			GPackages[ PackageName ].Class.push_back( ( UClass* ) Object );

			// I don't think this is capturing everything...

			for( UProperty* Child = ( UProperty* )( ( UClass* ) Object )->Children; Child; Child = ( UProperty* ) Child->Next )
			{
				if( Child->IsA( UFunction::StaticClass() ) )
				{
					FFunctionInfo F;

					F.Class		= ( UClass* ) Object;
					F.Function	= ( UFunction* ) Child;

					GPackages[ PackageName ].Function.push_back( F );
				}
			}
		}
		else if( Object->IsA( UStruct::StaticClass() ) )
		{
			// Maybe somehow make sure its not a function ;p

			if( Object->IsA( UFunction::StaticClass() ) == false )
			{
				GPackages[ PackageName ].Struct.push_back( ( UStruct* ) Object );
			}
		}
	}

	GApp.AddToLogFileA( "SDK.log", "Generating individual package files.." );

	for( FPackageMap::iterator i( GPackages.begin() ), j( GPackages.end() ); i != j; ++i )
	{
		std::wstring	PackageName = (*i).first;
		FPackage		Package		= (*i).second;
		std::string		Filename	= GApp.GetDirectoryFileFormatA( "SDK\\%S.h", PackageName.c_str() );

		if( !Package.Constant.size()	&& 
			!Package.Enum.size()		&& 
			!Package.Function.size()	&&
			!Package.Struct.size()		&&
			!Package.Class.size() )
		{
			GApp.AddToLogFileA( "error.log", "Package [%S] has no data, skipping...", PackageName.c_str() );

			continue;
		}

		FILE* FilePointer = fopen( Filename.c_str(), "a+" );

		if( GenerateHeader( PackageName, FilePointer ) == FALSE )
		{
			GApp.AddToLogFileA( "error.log", "Failed to generate Package header [%S]", PackageName.c_str() );
		}

		GApp.AddToLogFileA( "packages.log", "[%S]", PackageName.c_str() );

		fprintf( FilePointer, 
			"/////////////////////////////////////////////\n"
			"// Constants\n"
			"/////////////////////////////////////////////\n\n" );

		for( size_t Index = 0; Index < Package.Constant.size(); Index++ )
		{
			GApp.AddToLogFileA( "packages.log", "Const [%S]", Package.Constant[ Index ]->GetFullName().c_str() );

			if( GenerateConst( Package.Constant[ Index ], FilePointer ) == FALSE )
			{
				GApp.AddToLogFileA( "error.log", "Failed to generate UConst [%S]", Package.Constant[ Index ]->GetName().c_str() );
			}
		}

		if( Package.Constant.size() )
		{
			fprintf( FilePointer, "\n" );
		}

		fprintf( FilePointer, 
			"/////////////////////////////////////////////\n"
			"// Enums\n"
			"/////////////////////////////////////////////\n\n" );

		for( size_t Index = 0; Index < Package.Enum.size(); Index++ )
		{
			GApp.AddToLogFileA( "packages.log", "Enum [%S]", Package.Enum[ Index ]->GetFullName().c_str() );

			if( GenerateEnum( Package.Enum[ Index ], FilePointer ) == FALSE )
			{
				GApp.AddToLogFileA( "error.log", "Failed to generate UEnum [%S]", Package.Enum[ Index ]->GetName().c_str() );
			}
		}

		fprintf( FilePointer, 
			"/////////////////////////////////////////////\n"
			"// Structs\n"
			"/////////////////////////////////////////////\n\n" );

		for( size_t Index = 0; Index < Package.Struct.size(); Index++ )
		{
			GApp.AddToLogFileA( "packages.log", "Struct [%S]", Package.Struct[ Index ]->GetFullName().c_str() );

			if( GenerateStruct( Package.Struct[ Index ], FilePointer ) == FALSE )
			{
				GApp.AddToLogFileA( "error.log", "Failed to generate Struct [%S]", Package.Struct[ Index ]->GetName().c_str() );
			}
		}

		fprintf( FilePointer, 
			"/////////////////////////////////////////////\n"
			"// Functions\n"
			"/////////////////////////////////////////////\n\n" );

		for( size_t Index = 0; Index < Package.Function.size(); Index++ )
		{
			GApp.AddToLogFileA( "packages.log", "Function [%S]", Package.Function[ Index ].Function->GetFullName().c_str() );

			if( GenerateFunction( &Package.Function[ Index ], FilePointer ) == FALSE )
			{
				GApp.AddToLogFileA( "error.log", "Failed to generate UFunction [%S]", Package.Class[ Index ]->GetName().c_str() );
			}
		}

		fprintf( FilePointer, 
			"/////////////////////////////////////////////\n"
			"// Classes\n"
			"/////////////////////////////////////////////\n\n" );

		for( size_t Index = 0; Index < Package.Class.size(); Index++ )
		{
			GApp.AddToLogFileA( "packages.log", "Class [%S]", Package.Class[ Index ]->GetFullName().c_str() );

			if( GenerateClass( Package.Class[ Index ], FilePointer ) == FALSE )
			{
				GApp.AddToLogFileA( "error.log", "Failed to generate UClass [%S]", Package.Class[ Index ]->GetName().c_str() );
			}
		}

		if( GenerateFooter( FilePointer ) == FALSE )
		{
			GApp.AddToLogFileA( "error.log", "Failed to generate Package footer [%S]", PackageName.c_str() );
		}

		fclose( FilePointer );
	}

	// Make file that links all the packages together...

	GApp.AddToLogFileA( "SDK.log", "Generating link file.." );

	FILE* Link = fopen( GApp.GetDirectoryFileA( "SDK\\UE3GeneratorMain.h" ), "a+" );

	if( Link == NULL )
	{
		GApp.AddToLogFileA( "error.log", "Failed to finalize master file (Header that links headers)" );

		return;
	}

	fprintf( Link,
		"#ifndef __HEADER_UE3_MASTER__\n"
		"#define __HEADER_UE3_MASTER__\n\n" );

	fprintf( Link,
		"\n"
		"/////////////////////////////////////////////\n"
		"// Packages\n"
		"/////////////////////////////////////////////\n" );

	for( FPackageMap::iterator i( GPackages.begin() ), j( GPackages.end() ); i != j; ++i )
	{
		fprintf( Link, "#include \"%S.h\"\n", (*i).first.c_str() );
	}

	fprintf( Link,
		"/////////////////////////////////////////////\n"
		"// Prototype\n"
		"/////////////////////////////////////////////\n" );

	for( size_t i = 0; i < Prototype.size(); i++ )
	{
		// Copy prototypes

		CopyFileA( 
			GApp.GetDirectoryFileFormatA( "Prototype\\%s", Prototype[ i ].cFileName ), 
			GApp.GetDirectoryFileFormatA( "SDK\\%s", Prototype[ i ].cFileName ), FALSE );

		// Add prototype to link file

		fprintf( Link, "#include \"%s\"\n", Prototype[ i ].cFileName );
	}

	fprintf( Link, "\n#endif" );

	fclose( Link );
}

DWORD WINAPI lpUnrealSDKGenerator( LPVOID lpParam )
{
	while( true )
	{
		if( GetAsyncKeyState( VK_F1 ) & 1 )
		{
			GApp.AddToLogFileA( "SDK.log", "--- s0beit UE3 SDK Generator ---" );
			GApp.AddToLogFileA( "SDK.log", "--- Executable (%s) ---", GetExecutableName() );

			GenerateHeaders();
			
			GApp.AddToLogFileA( "SDK.log", "--- Generating Complete ---" );
		}

		Sleep( 100 );
	}

	return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD dwReason, LPVOID lpReserved )
{
	if( dwReason == DLL_PROCESS_ATTACH )
	{
		GApp.BaseUponModule( hModule );

		CreateThread( 0, 0, lpUnrealSDKGenerator, 0, 0, 0 );
	}

    return TRUE;
}