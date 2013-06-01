#include "stdafx.h"

#include <sstream>

#include <boost/regex.hpp>

#include "kdlib/exceptions.h"
#include "kdlib/dbgengine.h"
#include "kdlib/module.h"
#include "kdlib/typeinfo.h"

#include "typeinfoimp.h"

namespace {

///////////////////////////////////////////////////////////////////////////////

static const boost::wregex complexSymMatch(L"^([\\*]*)([^\\(\\)\\*\\[\\]]*)([\\(\\)\\*\\[\\]\\d]*)$");

std::wstring getTypeNameFromComplex( const std::wstring &fullTypeName )
{
    boost::wsmatch    matchResult;

    if ( !boost::regex_match( fullTypeName, matchResult, complexSymMatch ) )
        return L"";

    return std::wstring( matchResult[2].first, matchResult[2].second );
}

std::wstring getTypeSuffixFromComplex( const std::wstring &fullTypeName )
{
    boost::wsmatch    matchResult;

    if ( !boost::regex_match( fullTypeName, matchResult, complexSymMatch ) )
        return L"";

    return std::wstring( matchResult[3].first, matchResult[3].second );
}


///////////////////////////////////////////////////////////////////////////////

static const boost::wregex bracketMatch(L"^([^\\(]*)\\((.*)\\)([^\\)]*)$"); 

void getBracketExpression( std::wstring &suffix, std::wstring &bracketExpression )
{
    boost::wsmatch    matchResult;

    if ( boost::regex_match( suffix, matchResult, bracketMatch  ) )
    {
        bracketExpression = std::wstring( matchResult[2].first, matchResult[2].second );
        
        std::wstring  newSuffix;

        if ( matchResult[1].matched )
            newSuffix += std::wstring( matchResult[1].first, matchResult[1].second );

        if ( matchResult[3].matched )
            newSuffix += std::wstring( matchResult[3].first, matchResult[3].second );

        suffix = newSuffix;
    }
}

///////////////////////////////////////////////////////////////////////////////

static const boost::wregex ptrMatch(L"^\\*(.*)$");

bool getPtrExpression( std::wstring &suffix )
{
    boost::wsmatch    matchResult;

    if ( boost::regex_match( suffix, matchResult, ptrMatch  ) )
    {
        suffix = std::wstring(matchResult[1].first, matchResult[1].second );

        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

static const boost::wregex arrayMatch(L"^(.*)\\[(\\d+)\\]$");

bool getArrayExpression( std::wstring &suffix, size_t &arraySize )
{
    boost::wsmatch    matchResult;

    if ( boost::regex_match( suffix, matchResult, arrayMatch  ) )
    {
        std::wstringstream  sstr;
        sstr << std::wstring(matchResult[2].first, matchResult[2].second );
        sstr >> arraySize;

        suffix = std::wstring(matchResult[1].first, matchResult[1].second );

        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////

} // end nameless namespace

namespace kdlib {

///////////////////////////////////////////////////////////////////////////////

size_t getSymbolSize( const std::wstring &fullName )
{
    if ( TypeInfo::isBaseType( fullName ) )
        return TypeInfo::getBaseTypeInfo( fullName, ptrSize() )->getSize();

    std::wstring     moduleName;
    std::wstring     symName;

    splitSymName( fullName, moduleName, symName );

    ModulePtr  module;

    if ( moduleName.empty() )
    {
        MEMOFFSET_64 moduleOffset = findModuleBySymbol( symName );
        module = loadModule( moduleOffset );
    }
    else
    {
        module = loadModule( moduleName );
    }

    return module->getSymbolSize( symName );
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 getSymbolOffset( const std::wstring &fullName )
{
    std::wstring     moduleName;
    std::wstring     symName;

    splitSymName( fullName, moduleName, symName );

    ModulePtr  module;

    if ( moduleName.empty() )
    {
        MEMOFFSET_64 moduleOffset = findModuleBySymbol( symName );
        module = loadModule( moduleOffset );
    }
    else
    {
        module = loadModule( moduleName );
    }

    return module->getSymbolVa( symName );
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr loadType( const std::wstring &typeName )
{
    std::wstring     moduleName;
    std::wstring     symName;

    if ( TypeInfo::isBaseType( typeName ) )
        return TypeInfo::getBaseTypeInfo( typeName, ptrSize() );

    splitSymName( typeName, moduleName, symName );

    ModulePtr  module;

    if ( moduleName.empty() )
    {
        MEMOFFSET_64 moduleOffset = findModuleBySymbol( symName );
        module = loadModule( moduleOffset );
    }
    else
    {
        module = loadModule( moduleName );
    }

    SymbolPtr  symbolScope = module->getSymbolScope();

    return loadType(symbolScope, symName );
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr loadType(  SymbolPtr &symbolScope, const std::wstring &symbolName )
{
    if ( TypeInfo::isComplexType( symbolName ) )
        return TypeInfo::getComplexTypeInfo( symbolName, symbolScope );

     SymbolPtr  symbol  = symbolScope->getChildByName( symbolName );

     return loadType(symbol);
}


///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr loadType( SymbolPtr &symbol )
{
    unsigned long symTag = symbol->getSymTag();
    TypeInfoPtr  ptr;

    switch( symTag )
    {
    case SymTagData:

        if ( symbol->getLocType() == LocIsBitField )
        {
            ptr = TypeInfoPtr( new TypeInfoBitField(symbol) );
            break;
        }

        if ( symbol->getDataKind() == DataIsConstant )
        {
            NumVariant     constVal;
            symbol->getValue( constVal );  
            
            ptr = loadType( symbol->getType() );

            dynamic_cast<TypeInfoImp*>( ptr.get() )->setConstant( constVal );

            break;
        }

       return loadType( symbol->getType() );

    case SymTagBaseType:
        return TypeInfo::getBaseTypeInfo( symbol );

    case SymTagUDT:
    case SymTagBaseClass:
       return TypeInfoPtr( new TypeInfoUdt( symbol ) );

    case SymTagArrayType:
       return TypeInfoPtr( new TypeInfoArray( symbol ) );

    case SymTagPointerType:
        return TypeInfoPtr( new TypeInfoPointer( symbol ) );

    //case SymTagVTable:
    //    ptr = TypeInfoPtr( new PointerTypeInfo( typeSym->getType() ) );
    //    break;

    case SymTagEnum:
        ptr = TypeInfoPtr( new TypeInfoEnum( symbol ) );
        break;

    case SymTagTypedef:
        ptr = loadType( symbol->getType() );
        break;

    default:
        throw TypeException( L"",  L"this type is not supported" );
    }

    //if ( ptr )
    //    ptr->m_ptrSize = getTypePointerSize(typeSym);

    return ptr;
}

///////////////////////////////////////////////////////////////////////////////

static const boost::wregex baseMatch(L"^(Char)|(WChar)|(Int1B)|(UInt1B)|(Int2B)|(UInt2B)|(Int4B)|(UInt4B)|(Int8B)|(UInt8B)|(Long)|(ULong)|(Float)|(Bool)|(Double)|(Void)$" );

bool TypeInfo::isBaseType( const std::wstring &typeName )
{
    std::wstring  name = typeName;

    if ( isComplexType(name) )
    {
        name = getTypeNameFromComplex(name);
        if ( name.empty() )
            throw TypeException( typeName, L"invalid type name" );
    }

    boost::wsmatch    baseMatchResult;
    return boost::regex_match( name, baseMatchResult, baseMatch );
}

///////////////////////////////////////////////////////////////////////////////

bool TypeInfo::isComplexType( const std::wstring &typeName )
{
    return typeName.find_first_of( L"*[" ) !=  std::string::npos;
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfo::getBaseTypeInfo( const std::wstring &typeName, size_t ptrSize  )
{
    if ( isComplexType( typeName ) )
        return getComplexTypeInfo( typeName, SymbolPtr() );

    boost::wsmatch    baseMatchResult;

    if ( boost::regex_match( typeName, baseMatchResult, baseMatch ) )
    {
        if ( baseMatchResult[1].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<char>(L"Char", ptrSize) );

        if ( baseMatchResult[2].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<wchar_t>(L"WChar", ptrSize) );

        if ( baseMatchResult[3].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<char>(L"Int1B", ptrSize ) );

        if ( baseMatchResult[4].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<unsigned char>(L"UInt1B", ptrSize ) );

        if ( baseMatchResult[5].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<short>(L"Int2B", ptrSize) );

        if ( baseMatchResult[6].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<unsigned short>(L"UInt2B", ptrSize) );

        if ( baseMatchResult[7].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<long>(L"Int4B", ptrSize) );

        if ( baseMatchResult[8].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<unsigned long>(L"UInt4B", ptrSize) );

        if ( baseMatchResult[9].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<__int64>(L"Int8B", ptrSize) );

        if ( baseMatchResult[10].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<unsigned __int64>(L"UInt8B", ptrSize) );

        if ( baseMatchResult[11].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<long>(L"Long", ptrSize) );

        if ( baseMatchResult[12].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<unsigned long>(L"ULong", ptrSize) );

        if ( baseMatchResult[13].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<float>(L"Float", ptrSize) );

        if ( baseMatchResult[14].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<bool>(L"Bool", ptrSize) );

        if ( baseMatchResult[15].matched )
            return TypeInfoPtr( new TypeInfoBaseWrapper<double>(L"Double", ptrSize) );

        if ( baseMatchResult[16].matched )
            return TypeInfoPtr( new TypeInfoVoid( ptrSize ) );
   }

    NOT_IMPLEMENTED();

    //return TypeInfoPtr();
}

/////////////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfo::getBaseTypeInfo( SymbolPtr &symbol )  
{
    std::wstring  symName = getBasicTypeName( symbol->getBaseType() );

    if ( symName == L"Int" || symName == L"UInt" ) 
    {
        std::wstringstream   sstr;
        sstr << symName << symbol->getSize() << L"B";
        symName = sstr.str();
    }
    else if ( symName == L"Long" )
    {
        symName = L"Int4B";
    }
    else if ( symName == L"ULong" )
    {
        symName = L"UInt4B";
    }
    else if ( symName == L"Float" && symbol->getSize() == 8  )
    {
        symName = L"Double";
    }

    return getBaseTypeInfo( symName, getPtrSizeBySymbol(symbol) );
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfo::getComplexTypeInfo( const std::wstring &typeName, SymbolPtr &symbolScope )
{
    std::wstring  innerSymName = getTypeNameFromComplex(typeName);
    if ( innerSymName.empty() )
        throw TypeException( typeName, L"invalid type name" );

    std::wstring symSuffix = getTypeSuffixFromComplex(typeName);
    if ( symSuffix.empty() )
        throw TypeException( typeName, L"invalid type name" );

    size_t  pointerSize;
    if ( symbolScope )
        pointerSize = symbolScope->getMachineType() == machine_AMD64 ? 8 : 4;
    else
        pointerSize = ptrSize();

    if ( isBaseType(innerSymName) )
    {
        TypeInfoPtr    basePtr = getBaseTypeInfo( innerSymName, pointerSize );
        return getRecursiveComplexType( typeName, basePtr, symSuffix, pointerSize );
    }

    SymbolPtr lowestSymbol = symbolScope->getChildByName( innerSymName );

    if ( lowestSymbol->getSymTag() == SymTagData )
    {
        throw TypeException( typeName, L"symbol name can not be an expresion" );
    }

    TypeInfoPtr lowestType = loadType( lowestSymbol );
   
    return getRecursiveComplexType( typeName, lowestType, symSuffix, pointerSize );

}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfo::getRecursiveComplexType( const std::wstring &typeName, TypeInfoPtr &lowestType, std::wstring &suffix,  size_t ptrSize  )
{
    std::wstring bracketExpr;
    getBracketExpression( suffix, bracketExpr );

    while( !suffix.empty() )
    {
        if ( getPtrExpression( suffix ) )
        {
            lowestType = TypeInfoPtr( new TypeInfoPointer( lowestType, ptrSize ) );
            continue;
        }

        size_t arraySize;
        if ( getArrayExpression( suffix, arraySize ) )
        {
            lowestType = TypeInfoPtr( new TypeInfoArray( lowestType, arraySize ) );
            continue;
        }

        throw TypeException( typeName, L"symbol name can not be an expresion" );
    }

    if ( !bracketExpr.empty() )
        return getRecursiveComplexType( typeName, lowestType, bracketExpr, ptrSize );

    return lowestType;
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoImp::ptrTo()
{
    return TypeInfoPtr( new TypeInfoPointer( shared_from_this(), getPtrSize() ) );
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoImp::arrayOf( size_t size )
{
    return TypeInfoPtr( new TypeInfoArray( shared_from_this(), size ) );
}

///////////////////////////////////////////////////////////////////////////////

std::wstring TypeInfoReference::getName()
{
    std::wstring       name;
    TypeInfo          *typeInfo = this;

    std::wstring tiName;
    do {

        if ( typeInfo->isArray() )
        {
            std::vector<size_t>  indices;

            do {
                indices.push_back( typeInfo->getElementCount() );
            }
            while( ( typeInfo = dynamic_cast<TypeInfoArray*>(typeInfo)->getDerefType().get() )->isArray() );

            if ( !name.empty() )
            {
                name.insert( 0, 1, L'(' );
                name.insert( name.size(), 1, L')' );
            }

            std::wstringstream  sstr;

            for ( std::vector<size_t>::iterator  it = indices.begin(); it != indices.end(); ++it )
                sstr << L'[' << *it << L']';

            name += sstr.str();

            continue;
        }
        else
        if ( typeInfo->isPointer() )
        {
            name.insert( 0, 1, L'*' );

            TypeInfoPointer *ptrTypeInfo = dynamic_cast<TypeInfoPointer*>(typeInfo);
  /*          if (!ptrTypeInfo->derefPossible())
            {
                tiName = ptrTypeInfo->getDerefName();
                break;
            }*/

            typeInfo = ptrTypeInfo->deref().get();

            continue;
        }

        tiName = typeInfo->getName();
        break;

    } while ( true );

    name.insert( 0, tiName );

    return name;
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoArray::getElement( size_t index )
{
    if ( index >= m_count )
        throw IndexException( index );

    return m_derefType;
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoFields::getElement( const std::wstring &name )
{
    checkFields();

    size_t  pos = name.find_first_of( L'.');

    TypeInfoPtr  fieldType = m_fields.lookup( std::wstring( name, 0, pos) )->getTypeInfo();

    if ( pos == std::wstring::npos )
        return fieldType;

    return fieldType->getElement( std::wstring( name, pos + 1 ) );
}

///////////////////////////////////////////////////////////////////////////////

TypeInfoPtr TypeInfoFields::getElement(size_t index )
{
    checkFields();

    return m_fields.lookup(index)->getTypeInfo();
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_32 TypeInfoFields::getElementOffset( const std::wstring &name )
{
    checkFields();

    size_t  pos = name.find_first_of( L'.');

    MEMOFFSET_32  offset = m_fields.lookup( std::wstring( name, 0, pos) )->getOffset();

    if ( pos != std::wstring::npos )
    {
        TypeInfoPtr  fieldType = getElement( std::wstring( name, 0, pos ) );

        offset += fieldType->getElementOffset( std::wstring( name, pos + 1 ) );
    }

    return offset;
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_32 TypeInfoFields::getElementOffset( size_t index )
{
    checkFields();

    return m_fields.lookup( index )->getOffset();
}

///////////////////////////////////////////////////////////////////////////////

size_t TypeInfoFields::getElementCount()
{
    checkFields();

    return m_fields.count();
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 TypeInfoFields::getElementVa( const std::wstring &name )
{
    checkFields();

    size_t  pos = name.find_first_of( L'.');

    UdtFieldPtr  fieldPtr = m_fields.lookup( std::wstring( name, 0, pos) );

    if ( pos == std::wstring::npos )
        return fieldPtr->getStaticOffset();

    TypeInfoPtr  fieldType = fieldPtr->getTypeInfo();

    return fieldType->getElementVa( std::wstring( name, pos + 1 ) );
}

///////////////////////////////////////////////////////////////////////////////

MEMOFFSET_64 TypeInfoFields::getElementVa( size_t index )
{
   checkFields();

   return m_fields.lookup( index )->getStaticOffset();
}

///////////////////////////////////////////////////////////////////////////////

bool TypeInfoFields::isStaticMember( const std::wstring &name )
{
    checkFields();

    size_t  pos = name.find_first_of( L'.');

    UdtFieldPtr  fieldPtr = m_fields.lookup( std::wstring( name, 0, pos) );

    if ( pos == std::wstring::npos )
        return fieldPtr->isStaticMember();

    TypeInfoPtr  fieldType = fieldPtr->getTypeInfo();

    return fieldType->isStaticMember( std::wstring( name, pos + 1 ) );
}

///////////////////////////////////////////////////////////////////////////////

bool TypeInfoFields::isStaticMember( size_t index )
{
    checkFields();

    return m_fields.lookup( index )->isStaticMember();
}

///////////////////////////////////////////////////////////////////////////////

void TypeInfoUdt::getFields()
{
    getFields( m_symbol, SymbolPtr() );
    getVirtualFields();
}

///////////////////////////////////////////////////////////////////////////////

void TypeInfoUdt::getFields( 
        SymbolPtr &rootSym, 
        SymbolPtr &baseVirtualSym,
        MEMOFFSET_32 startOffset,
        MEMOFFSET_32 virtualBasePtr,
        size_t virtualDispIndex,
        size_t virtualDispSize )
{
    size_t   childCount = rootSym->getChildCount();

    for ( size_t  i = 0; i < childCount; ++i )
    {
        SymbolPtr  childSym = rootSym->getChildByIndex( i );

        SymTags  symTag = childSym->getSymTag();

        if ( symTag == SymTagBaseClass )
        {
            if ( !childSym->isVirtualBaseClass() )
            {
                getFields( childSym, SymbolPtr(), startOffset + childSym->getOffset() );
            }
        }
        else
        if ( symTag == SymTagData )
        {
            UdtFieldPtr  fieldPtr;

            switch ( childSym->getDataKind() )
            {
            case DataIsMember:
                fieldPtr = SymbolUdtField::getField( childSym, childSym->getName(), startOffset + childSym->getOffset(), virtualBasePtr, virtualDispIndex, virtualDispSize );
                break;
            case DataIsStaticMember:
                fieldPtr = SymbolUdtField::getStaticField( childSym, childSym->getName(),childSym->getVa() );
                break;
            }

            m_fields.push_back( fieldPtr );
        }
        else
        if ( symTag == SymTagVTable )
        {
            UdtFieldPtr  fieldPtr = SymbolUdtField::getField(  childSym, L"__VFN_table", startOffset + childSym->getOffset(), virtualBasePtr, virtualDispIndex, virtualDispSize );

            m_fields.push_back( fieldPtr );
        }
    }  
}

///////////////////////////////////////////////////////////////////////////////

void TypeInfoUdt::getVirtualFields()
{
    size_t   childCount = m_symbol->getChildCount(SymTagBaseClass);

    for ( size_t i = 0; i < childCount; ++i )
    {
        SymbolPtr  childSym = m_symbol->getChildByIndex( i );

        if ( !childSym->isVirtualBaseClass() )
            continue;

        getFields( 
            childSym,
            childSym,
            0,
            childSym->getVirtualBasePointerOffset(),
            childSym->getVirtualBaseDispIndex(),
            childSym->getVirtualBaseDispSize() );
    }
}

///////////////////////////////////////////////////////////////////////////////

void TypeInfoEnum::getFields()
{
    size_t   childCount = m_symbol->getChildCount();

    for ( size_t  i = 0; i < childCount; ++i )
    {
        SymbolPtr  childSym = m_symbol->getChildByIndex( i );

        UdtFieldPtr  fieldPtr = UdtFieldPtr( new EnumField( childSym ) );

        m_fields.push_back( fieldPtr );
    }
}

///////////////////////////////////////////////////////////////////////////////


TypeInfoBitField::TypeInfoBitField( SymbolPtr &symbol )
{
    m_bitWidth = symbol->getSize();
    m_bitPos = symbol->getBitPosition();
    m_bitType = TypeInfo::getBaseTypeInfo( symbol->getType() );
    m_size = m_bitType->getSize();
}

///////////////////////////////////////////////////////////////////////////////

std::wstring TypeInfoBitField::getName() 
{
    std::wstringstream   sstr;

    sstr << m_bitType->getName()  << L":" <<  m_bitWidth;
    return  sstr.str();
}

///////////////////////////////////////////////////////////////////////////////



} // kdlib namespace end