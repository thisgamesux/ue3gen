//Start including the nessicary UE Core files...this includes stuff like the data needed to walk the tables and such

#ifndef _SDK_MAIN_
#define _SDK_MAIN_

#define GObjects                                   0x01DE5FA4
#define GNames                                     0x01DD46E8

#define CPF_Edit									0x1
#define CPF_Const									0x2
#define CPF_OptionalParm							0x10
#define CPF_Net										0x20
#define CPF_Parm									0x80
#define CPF_OutParm									0x100
#define CPF_ReturnParm								0x400
#define CPF_CoerceParm								0x800
#define CPF_Native									0x1000
#define CPF_Transient								0x2000
#define CPF_Config									0x4000
#define CPF_Localized								0x8000

template < class T > struct TArray
{
public:
	T*	Data;
	int Count;
	int Max;

public:
	int Num ()
	{
		return Count;
	};

	T at ( int Index )
	{
		return Data[ Index ];
	};
};

class FNameEntry
{
public:
	__int32			Index;
	__int32			Unknown001;
	__int32			Unknown002;
	__int32			Unknown003;
	wchar_t			Name[ 16 ];
};

class FName
{
public:
	int				Index;
	unsigned char	unknown_data00[ 4 ];

public:
	static TArray< FNameEntry* >* Names ()
	{
		return ((TArray<FNameEntry *>*)GNames);
	};

	wchar_t* GetName ()
	{
		TArray< FNameEntry* >* pNames = Names();

		if( pNames == NULL ) return NULL;

		FNameEntry *pNameEntry = pNames->at( Index );

		if( pNameEntry == NULL ) return NULL;

		return pNameEntry->Name;
	};
};

class FString : public TArray< wchar_t >
{
public:
	FString ()
	{
	};

	FString ( wchar_t *s )
	{
		Set( s );
	}

	~FString ()
	{
	};

	void Set ( wchar_t* Buffer )
	{
		this->Data	= Buffer;
		this->Count = this->Max = static_cast< int >( wcslen( Buffer ) ) + 1;
	};
};

struct FScriptDelegate
{
	unsigned char unknowndata00[ 10 ];
};

struct FIntPoint
{
	int                                                X;
	int                                                Y;
};

struct FPointer
{
	int                                                Dummy;
};

struct FDouble
{
	int                                                A;
	int                                                B;
};

struct FQWord
{
	int                                                A;
	int                                                B;
};

class UObject
{
public:
	int													VfTableObject;					//0000
	int													ObjectInternalInteger;			//0004
	struct FQWord										ObjectFlags;					//0008
	struct FPointer										HashNext;						//0010
	struct FPointer										HashOuterNext;					//0014
	struct FPointer										StateFrame;						//0018
	class UObject*										Linker;							//001C
	struct FPointer										LinkerIndex;					//0020
	int													NetIndex;						//0024
	class UObject*										Outer;							//0028
	class FName											Name;							//002C
	class UClass*										Class;							//0034
	class UObject*										ObjectArchetype;				//0038

	static UClass* StaticClass ()
	{
		static UClass* ClassPointer = NULL;

		if ( ClassPointer == NULL )
		{
			ClassPointer = ( UClass* )UObject::FindObject( "Class Core.Object" );
		}

		return ClassPointer;
	};

public:
	static TArray< UObject* >* GObjObjects ();

	std::wstring GetName();
	std::wstring GetNameCPP();
	std::wstring GetFullName();
	std::wstring GetPackageName();

	bool IsA ( UClass* pClass );

	static UObject* FindObject ( wchar_t* pObjectName );
	static UObject* FindObject ( char* pObjectName );
};

class UField : public UObject
{
public:
	class UField*                                      SuperField;
	class UField*                                      Next;

public:
	static UClass* StaticClass ()
	{
		static UClass *ClassPointer = NULL;

		if ( !ClassPointer )
			ClassPointer = ( UClass* )UObject::FindObject( "Class Core.Field" );

		return ClassPointer;
	};
};

class UEnum : public UField
{
public:
	TArray< FName > Names;				

public:
	static UClass* StaticClass( void )
	{
		static UClass* PrivStaticClass = NULL;

		if ( !PrivStaticClass )
			PrivStaticClass = ( UClass* ) FindObject( "Class Core.Enum" );

		return PrivStaticClass;
	}
};

class UConst : public UField
{
public:
	FString Value;

public:
	static UClass* StaticClass( void )
	{
		static UClass* PrivStaticClass = NULL;

		if ( !PrivStaticClass )
			PrivStaticClass = ( UClass* ) FindObject( "Class Core.Const" );

		return PrivStaticClass;
	};
};

class UStruct : public UField
{
public:
	DWORD			ScriptText;					// 0x48
	DWORD			CppText;					// 0x4C
	UField*			Children;					// 0x50
	DWORD			PropertySize;				// 0x54
	TArray< BYTE >	Script;						// 0x58
	unsigned char	unknown_data0x1[0x30];	// 0x60

public:
	static UClass* StaticClass( void )
	{
		static UClass* PrivStaticClass = NULL;

		if ( !PrivStaticClass )
			PrivStaticClass = ( UClass* ) FindObject( "Class Core.Struct" );

		return PrivStaticClass;
	};
};

class UFunction : public UStruct
{
public:
	DWORD					FunctionFlags;
	WORD					iNative;
	WORD					RepOffset;
	BYTE					OperPrecedence;
	FName					FriendlyName;
	BYTE					NumParms;
	WORD					ParmsSize;
	WORD					ReturnValueOffset;
	class UStructProperty*	FirstStructWithDefaults;

public:
	static UClass* StaticClass( void )
	{
		static UClass* PrivStaticClass = NULL;

		if ( !PrivStaticClass )
			PrivStaticClass = ( UClass* ) FindObject( "Class Core.Function" );

		return PrivStaticClass;
	};
};

class UState : public UStruct
{
public:
	unsigned char                                      unknown_data24[ 44 ];

public:
	static UClass* StaticClass ()
	{
		static UClass *ClassPointer = NULL;

		if ( !ClassPointer )
			ClassPointer = ( UClass* )UObject::FindObject( "Class Core.State" );

		return ClassPointer;
	};
};

class UClass : public UState
{
public:
	unsigned char                                      unknown_data25[ 192 ];

public:
	static UClass* StaticClass ()
	{
		static UClass *ClassPointer = NULL;

		if ( !ClassPointer )
			ClassPointer = ( UClass* )UObject::FindObject( "Class Core.Class" );

		return ClassPointer;
	};
};

class UProperty : public UField 
{
public:
	DWORD ArrayDim;						
	DWORD ElementSize;
	DWORD PropertyFlags;
	DWORD PropertyFlags2;
	char unknown_data[0x10];
	DWORD PropertyOffset;
	char unknown_data0x1[0x1C];

public:
	static UClass* StaticClass( void )
	{
		static UClass* PrivStaticClass = NULL;

		if ( !PrivStaticClass )
			PrivStaticClass = ( UClass* )FindObject( "Class Core.Property" );

		return PrivStaticClass;
	}
};

class UByteProperty : public UProperty 
{
public:
	static UClass* StaticClass( void )
	{
		static UClass* PrivStaticClass = NULL;

		if ( !PrivStaticClass )
			PrivStaticClass = ( UClass* )FindObject( "Class Core.ByteProperty" );

		return PrivStaticClass;
	}
};

class UIntProperty : public UProperty 
{
public:
	static UClass* StaticClass( void )
	{
		static UClass* PrivStaticClass = NULL;

		if ( !PrivStaticClass )
			PrivStaticClass = ( UClass* )FindObject( "Class Core.IntProperty" );

		return PrivStaticClass;
	}
};

class UFloatProperty : public UProperty 
{
public:
	static UClass* StaticClass( void )
	{
		static UClass* PrivStaticClass = NULL;

		if ( !PrivStaticClass )
			PrivStaticClass = ( UClass* )FindObject( "Class Core.FloatProperty" );

		return PrivStaticClass;
	}
};

class UBoolProperty : public UProperty 
{
public:
	int Order;
public:
	static UClass* StaticClass( void )
	{
		static UClass* PrivStaticClass = NULL;

		if ( !PrivStaticClass )
			PrivStaticClass = ( UClass* )FindObject( "Class Core.BoolProperty" );

		return PrivStaticClass;
	}
};

class UStrProperty : public UProperty 
{
public:
	static UClass* StaticClass( void )
	{
		static UClass* PrivStaticClass = NULL;

		if ( !PrivStaticClass )
			PrivStaticClass = ( UClass* )FindObject( "Class Core.StrProperty" );

		return PrivStaticClass;
	}
};

class UNameProperty : public UProperty 
{
public:
	static UClass* StaticClass( void )
	{
		static UClass* PrivStaticClass = NULL;

		if ( !PrivStaticClass )
			PrivStaticClass = ( UClass* )FindObject( "Class Core.NameProperty" );

		return PrivStaticClass;
	}
};


class UObjectProperty : public UProperty 
{
public:
	UClass* PropertyClass;

	static UClass* StaticClass( void )
	{
		static UClass* PrivStaticClass = NULL;

		if ( !PrivStaticClass )
			PrivStaticClass = ( UClass* )FindObject( "Class Core.ObjectProperty" );

		return PrivStaticClass;
	}
};

class UClassProperty : public UProperty 
{
public:
	UClass* MetaClass;

	static UClass* StaticClass( void )
	{
		static UClass* PrivStaticClass = NULL;

		if ( !PrivStaticClass )
			PrivStaticClass = ( UClass* )FindObject( "Class Core.ClassProperty" );

		return PrivStaticClass;
	}
};

class UStructProperty : public UProperty 
{
public:
	UStruct* Struct;

	static UClass* StaticClass( void )
	{
		static UClass* PrivStaticClass = NULL;

		if ( !PrivStaticClass )
			PrivStaticClass = ( UClass* )FindObject( "Class Core.StructProperty" );

		return PrivStaticClass;
	}
};

class UArrayProperty : public UProperty 
{
public:
	UProperty* Inner;

	static UClass* StaticClass( void )
	{
		static UClass* PrivStaticClass = NULL;

		if ( !PrivStaticClass )
			PrivStaticClass = ( UClass* )FindObject( "Class Core.ArrayProperty" );

		return PrivStaticClass;
	}
};

class UDelegateProperty : public UProperty 
{
public:
	static UClass* StaticClass( void )
	{
		static UClass* PrivStaticClass = NULL;

		if ( !PrivStaticClass )
			PrivStaticClass = ( UClass* )FindObject( "Class Core.DelegateProperty" );

		return PrivStaticClass;
	}
};

class UPointerProperty : public UProperty 
{
public:
	static UClass* StaticClass( void )
	{
		static UClass* PrivStaticClass = NULL;

		if ( !PrivStaticClass )
			PrivStaticClass = ( UClass* )FindObject( "Class Core.PointerProperty" );

		return PrivStaticClass;
	}
};

#endif