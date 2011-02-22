//*****************************************************************************
//
// Microsoft Windows Media
// Copyright ( C) Microsoft Corporation. All rights reserved.
//
// FileName:            rostream.cpp
//
// Abstract:            Implementation of CROStream, which is an IStream-
//						derived data sourcing class. The IWMSyncReader 
//						interface is capable of sourcing through a custom object 
//						supporting the IStream interface. The CROStream class is 
//						used in the sample to demonstrate this capability.
//
//*****************************************************************************


#include "StdAfx.h"
#include "ROStream.h"

#include "../../../svplib/svplib.h"

#define SVP_LogMsg5 __noop
//////////////////////////////////////////////////////////////////////////////
//////

#if GDCLWMVFILTER

StreamOnAsyncReader::StreamOnAsyncReader(IAsyncReader* pRdr, LONGLONG llPos)
: CUnknown(NAME("StreamOnAsyncReader"), NULL),
m_pReader(pRdr),
m_llPosition(llPos)
{
}

STDMETHODIMP 
StreamOnAsyncReader::NonDelegatingQueryInterface(REFIID iid, void** ppv)
{
    if ((iid == IID_IStream) || (iid == IID_ISequentialStream))
    {
        return GetInterface((IStream*)this, ppv);
    }
    return CUnknown::NonDelegatingQueryInterface(iid, ppv);
}

STDMETHODIMP
StreamOnAsyncReader::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
    LONGLONG cLength, cAvail;
    HRESULT hr = m_pReader->Length(&cLength, &cAvail);
    if (FAILED(hr))
    {
        return hr;
    }
    if (m_llPosition >= cLength)
    {
        cb = 0;
    } else 
    {
        if ((m_llPosition + cb) > cLength)
        {
            cb = long(cLength - m_llPosition);
        }
        hr = m_pReader->SyncRead(m_llPosition, cb, (BYTE*)pv);
    }
    if (FAILED(hr))
    {
        cb = 0;
    }
    m_llPosition += cb;
    *pcbRead = cb;
    return hr;
}

STDMETHODIMP
StreamOnAsyncReader::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
    switch(dwOrigin)
    {
    case STREAM_SEEK_SET:
        m_llPosition = dlibMove.QuadPart;
        break;

    case STREAM_SEEK_CUR:
        m_llPosition += dlibMove.QuadPart;
        break;

    case STREAM_SEEK_END:
        {
            LONGLONG cLength, cAvail;
            HRESULT hr = m_pReader->Length(&cLength, &cAvail);
            if (FAILED(hr))
            {
                return hr;
            }
            m_llPosition = cLength + dlibMove.QuadPart;
            break;
        }
    default:
        return E_INVALIDARG;
    }
    if (plibNewPosition != NULL)
    {
        plibNewPosition->QuadPart = m_llPosition;
    }
    return S_OK;
}

STDMETHODIMP
StreamOnAsyncReader::Clone(IStream **ppstm)
{
    IStreamPtr pClone = new StreamOnAsyncReader(m_pReader, m_llPosition);
    *ppstm = pClone.Detach();
    return S_OK;
}

STDMETHODIMP
StreamOnAsyncReader::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    LONGLONG cLength, cAvail;
    HRESULT hr = m_pReader->Length(&cLength, &cAvail);
    if (FAILED(hr))
    {
        return hr;
    }

    ZeroMemory(pstatstg, sizeof(STATSTG));
    pstatstg->type = STGTY_STREAM;
    pstatstg->cbSize.QuadPart = cLength;
    return S_OK;
}


STDMETHODIMP
StreamOnAsyncReader::Write(const void *pv, ULONG cb, ULONG *pcbWritten)
{
    DbgLog((LOG_ERROR, 0, "IStream::Write called"));
    return E_NOTIMPL;
}

STDMETHODIMP
StreamOnAsyncReader::SetSize(ULARGE_INTEGER libNewSize)
{
    DbgLog((LOG_ERROR, 0, "IStream::SetSize called"));
    return E_NOTIMPL;
}

STDMETHODIMP
StreamOnAsyncReader::CopyTo(IStream *pstm, ULARGE_INTEGER cb,
                            ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
    DbgLog((LOG_ERROR, 0, "IStream::CopyTo called"));
    return E_NOTIMPL;
}

STDMETHODIMP
StreamOnAsyncReader::Commit(DWORD grfCommitFlags)
{
    DbgLog((LOG_ERROR, 0, "IStream::Commit called"));
    return E_NOTIMPL;
}

STDMETHODIMP
StreamOnAsyncReader::Revert()
{
    DbgLog((LOG_ERROR, 0, "IStream::Revert called"));
    return E_NOTIMPL;
}

STDMETHODIMP
StreamOnAsyncReader::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    DbgLog((LOG_ERROR, 0, "IStream::LockRegion called"));
    return E_NOTIMPL;
}

STDMETHODIMP
StreamOnAsyncReader::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    DbgLog((LOG_ERROR, 0, "IStream::UnlockRegion called"));
    return E_NOTIMPL;
}


#else

CROStream::CROStream(IAsyncReader* pAsyncReader) :
    m_cRefs( 1 ),
    m_pAsyncReader( 0 )
{
    m_pAsyncReader = pAsyncReader;
    m_pos = 0;
    LONGLONG llTotal, llAvalible;
    m_pAsyncReader->Length(&llTotal, &llAvalible);
    m_len = llTotal;
}

//////////////////////////////////////////////////////////////////////////////
/////
CROStream::~CROStream()
{
   m_pAsyncReader = NULL;
}

//////////////////////////////////////////////////////////////////////////////
///////
HRESULT CROStream::Read( void *pv, ULONG cb, ULONG *pcbRead )
{
    SVP_LogMsg5(L"  CROStream::Read %f %d", double(m_pos), cb);
    if(!m_pAsyncReader)
        return E_FAIL;
    
    UINT readed = cb;
   
    HRESULT hr = m_pAsyncReader->SyncRead(m_pos, cb, (BYTE*)pv);
    if(  FAILED(hr) )
    {
         SVP_LogMsg5(L"CROStream::Read Filed %x", hr);
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }
    m_pos+= cb;
    *pcbRead =readed;
    SVP_LogMsg5(L"  CROStream::Read End ");
    return( S_OK );
}

//////////////////////////////////////////////////////////////////////////////
///////
HRESULT CROStream::Seek(
                                    LARGE_INTEGER dlibMove,
                                    DWORD dwOrigin,
                                    ULARGE_INTEGER *plibNewPosition )
{
    SVP_LogMsg5(L"  CROStream::Seek %f %d %x",double(dlibMove.QuadPart), dwOrigin, m_pAsyncReader);
    if(!m_pAsyncReader)
        return E_FAIL;
    
    SVP_LogMsg5(L"  CROStream::Seek2 %d %x", dwOrigin, plibNewPosition);

    switch( dwOrigin )
    {
        case STREAM_SEEK_SET:
            SVP_LogMsg5(L"  CROStream::Seek3");
            m_pos = dlibMove.QuadPart;
            SVP_LogMsg5(L"  CROStream::Seek4");
            break;

        case STREAM_SEEK_CUR:
            m_pos += dlibMove.QuadPart;
            break;

        case STREAM_SEEK_END:
            m_pos = m_len + dlibMove.QuadPart;
            break;

        default:
            return( E_INVALIDARG );
    };

    SVP_LogMsg5(L"  CROStream::Seek5");
    //BYTE* ptrDummy;
    //m_pAsyncReader->SyncRead( dlibMove.QuadPart, 0,  ptrDummy);
    //plibNewPosition->QuadPart = (m_pAsyncReader->Seek(dlibMove.QuadPart, dwMoveMethod));
    if(plibNewPosition){
        plibNewPosition->QuadPart = m_pos;
        SVP_LogMsg5(L"  CROStream::Seek End %d %d", plibNewPosition->HighPart, plibNewPosition->LowPart);
    }else{
        SVP_LogMsg5(L"  CROStream::Seek6");
    }
    
    return( S_OK );
}

//////////////////////////////////////////////////////////////////////////////
///////
HRESULT CROStream::Stat( STATSTG *pstatstg, DWORD grfStatFlag )
{
    SVP_LogMsg5(L"  CROStream::Stat ");
    if(!m_pAsyncReader)
        return E_FAIL;

    if( ( NULL == pstatstg ) || ( STATFLAG_NONAME != grfStatFlag ) )
    {
        return( E_INVALIDARG );
    }

    LONGLONG llTotal, llAvalible;
    m_pAsyncReader->Length(&llTotal, &llAvalible);


    memset( pstatstg, 0, sizeof( STATSTG ) );

    pstatstg->type = STGTY_STREAM;
    pstatstg->cbSize.QuadPart = ( llTotal );

    SVP_LogMsg5(L"  CROStream::Stat ");
    return( S_OK );
}

//////////////////////////////////////////////////////////////////////////////
//  IUnknown
//////////////////////////////////////////////////////////////////////////////
HRESULT CROStream::QueryInterface( REFIID riid, void **ppv )
{
    if( ( IID_IUnknown == riid ) || ( IID_IStream == riid ) )
    {
        *ppv = this;
        AddRef();

        return( S_OK );
    }
        
    *ppv = NULL;
    return( E_NOINTERFACE );
}


//////////////////////////////////////////////////////////////////////////////
ULONG CROStream::AddRef()
{
    return( InterlockedIncrement( &m_cRefs ) );
}


//////////////////////////////////////////////////////////////////////////////
ULONG CROStream::Release()
{
    if( 0 == InterlockedDecrement( &m_cRefs ) )
    {
        delete this;
        return( 0 );
    }
    
    return( 0xbad );
}

#endif
