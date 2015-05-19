#include "stdafx.h"
#include "SDK_Main.h"

TArray< UObject* >* UObject::GObjObjects ()
{
	TArray< UObject* >* ObjectArray = ( TArray< UObject* >* )GObjects;

	if( ObjectArray == NULL )
		return NULL;

	return ObjectArray;
}

wstring UObject::GetName ()
{
	wchar_t *pszName = Name.GetName();

	if( pszName == NULL )
	{
		return wstring( L"" );
	}

	return wstring( pszName );
}

wstring UObject::GetNameCPP()
{
	wstring R;

	if( this->IsA( UClass::StaticClass() ) )
	{
		UClass *pClass = ( UClass* )this;

		while( pClass )
		{
			wstring className = pClass->GetName();

			if( className.compare( L"Actor" ) == 0 )
			{
				R = wstring( L"A" );

				break;
			}
			else if( className.compare( L"Object" ) == 0 )
			{
				R = wstring( L"U" );

				break;
			}

			pClass = ( UClass* ) pClass->SuperField;
		}
	}
	else
	{
		R = wstring( L"F" );
	}

	R += this->GetName();

	return R;
}

wstring UObject::GetFullName ()
{
	wstring R;

	if( this->Class && this->Outer )
	{
		R += this->Class->GetName();
		R += wstring( L" " );
		
		if( this->Outer->Outer )
		{
			R += this->Outer->Outer->GetName();
			R += wstring( L"." );
		}

		R += this->Outer->GetName();
		R += wstring( L"." );
		R += this->GetName();
	}

	return R;
}

wstring UObject::GetPackageName ()
{
	wstring R;

	if( !Outer )
	{
		R.clear();
	}
	else
	{
		if( Outer->Outer )
		{
			R = Outer->Outer->Name.GetName();
		}
		else
		{
			R = Outer->Name.GetName();
		}
	}

	return R;
}

UObject* UObject::FindObject ( wchar_t* ObjectName )
{
	for ( int i = 0; i < UObject::GObjObjects()->Count; ++i )
	{
		UObject *pObject = GObjObjects()->Data[ i ];

		if ( pObject == NULL )
			continue;

		if( pObject->GetName().compare( ObjectName ) == 0 )
			return pObject;

		wstring FullName = pObject->GetFullName();

		if( FullName.length() )
		{
			if ( FullName.compare( ObjectName ) == 0 )
			{
				return pObject;
			}
		}
	}

	return NULL;
}

UObject* UObject::FindObject ( char* pObjectName )
{
	string objectName = string( pObjectName );

	wstring temp( objectName.length(), L' ' );

	copy( objectName.begin(), objectName.end(), temp.begin() );

	return FindObject( ( wchar_t* ) temp.c_str() );
}

bool UObject::IsA( UClass* pClass )
{
	for ( UClass* SuperClass = this->Class; SuperClass; SuperClass = ( UClass* )SuperClass->SuperField )
	{
		if ( SuperClass == pClass )
		{
			return true;
		}
	}

	return false;
}