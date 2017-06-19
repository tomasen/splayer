// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

#pragma once

#ifndef __ATLCOMCLI_H__
#error AtlComCli.h should be included before including this file.
#endif

namespace ATL
{

template< typename BaseType, class StringTraits >
CStringT< BaseType, StringTraits >::CStringT( const VARIANT& varSrc ) :
	CThisSimpleString( StringTraits::GetDefaultManager() )
{
	CComVariant varResult;
	HRESULT hr = ::VariantChangeType( &varResult, const_cast< VARIANT* >( &varSrc ), 0, VT_BSTR );
	if( FAILED( hr ) )
	{
		AtlThrow( hr );
	}

	*this = V_BSTR( &varResult );
}

template< typename BaseType, class StringTraits >
CStringT< BaseType, StringTraits >::CStringT( const VARIANT& varSrc, IAtlStringMgr* pStringMgr ) :
	CThisSimpleString( pStringMgr )
{
	CComVariant varResult;
	HRESULT hr = ::VariantChangeType( &varResult, const_cast< VARIANT* >( &varSrc ), 0, VT_BSTR );
	if( FAILED( hr ) )
	{
		AtlThrow( hr );
	}

	*this = V_BSTR( &varResult );
}

template< typename BaseType, class StringTraits >
CStringT< BaseType, StringTraits >& CStringT< BaseType, StringTraits >::operator=( const VARIANT& var )
{
	CComVariant varResult;
	HRESULT hr = ::VariantChangeType( &varResult, const_cast< VARIANT* >( &var ), 0, VT_BSTR );
	if( FAILED( hr ) )
	{
		AtlThrow( hr );
	}

	*this = V_BSTR( &varResult );

	return( *this );
}

template< typename BaseType, class StringTraits >
CStringT< BaseType, StringTraits >& CStringT< BaseType, StringTraits >::operator+=( const VARIANT& var )
{
	CComVariant varResult;
	HRESULT hr = ::VariantChangeType( &varResult, const_cast< VARIANT* >( &var ), 0, VT_BSTR );
	if( FAILED( hr ) )
	{
		AtlThrow( hr );
	}

	*this += V_BSTR( &varResult );

	return( *this );
}


}	// namespace ATL
