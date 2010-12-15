/* 
*	Copyright (C) 2003-2006 Gabest
*	http://www.gabest.org
*
*  This Program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2, or (at your option)
*  any later version.
*   
*  This Program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*  GNU General Public License for more details.
*   
*  You should have received a copy of the GNU General Public License
*  along with GNU Make; see the file COPYING.  If not, write to
*  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
*  http://www.gnu.org/copyleft/gpl.html
*
*/

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

#include <afx.h>
#include <afxwin.h>         // MFC core and standard components
#include <atlcoll.h>

// TODO: reference additional headers your program requires here

#include <dshow.h>
#include <streams.h>
#include <dvdmedia.h>


#define MKTAG(a,b,c,d) (a | (b << 8) | (c << 16) | (d << 24))
#define MKBETAG(a,b,c,d) (d | (c << 8) | (b << 16) | (a << 24))
#define SCHl_TAG MKTAG('S', 'C', 'H', 'l')
#define SEAD_TAG MKTAG('S', 'E', 'A', 'D')    /* Sxxx header */
#define SNDC_TAG MKTAG('S', 'N', 'D', 'C')    /* Sxxx data */
#define SEND_TAG MKTAG('S', 'E', 'N', 'D')    /* Sxxx end */
#define SHEN_TAG MKTAG('S', 'H', 'E', 'N')    /* SxEN header */
#define SDEN_TAG MKTAG('S', 'D', 'E', 'N')    /* SxEN data */
#define SEEN_TAG MKTAG('S', 'E', 'E', 'N')    /* SxEN end */
#define ISNh_TAG MKTAG('1', 'S', 'N', 'h')    /* 1SNx header */
#define EACS_TAG MKTAG('E', 'A', 'C', 'S')
#define ISNd_TAG MKTAG('1', 'S', 'N', 'd')    /* 1SNx data */
#define ISNe_TAG MKTAG('1', 'S', 'N', 'e')    /* 1SNx end */
#define PT00_TAG MKTAG('P', 'T', 0x0, 0x0)
#define GSTR_TAG MKTAG('G', 'S', 'T', 'R')
#define SCDl_TAG MKTAG('S', 'C', 'D', 'l')
#define SCEl_TAG MKTAG('S', 'C', 'E', 'l')
#define kVGT_TAG MKTAG('k', 'V', 'G', 'T')    /* TGV i-frame */
#define fVGT_TAG MKTAG('f', 'V', 'G', 'T')    /* TGV p-frame */
#define mTCD_TAG MKTAG('m', 'T', 'C', 'D')    /* MDEC */
#define MADk_TAG MKTAG('M', 'A', 'D', 'k')    /* MAD i-frame */
#define MADm_TAG MKTAG('M', 'A', 'D', 'm')    /* MAD p-frame */
#define MADe_TAG MKTAG('M', 'A', 'D', 'e')    /* MAD lqp-frame */
#define MPCh_TAG MKTAG('M', 'P', 'C', 'h')    /* MPEG2 */
#define TGQs_TAG MKTAG('T', 'G', 'Q', 's')    /* TGQ i-frame (appears in .TGQ files) */
#define pQGT_TAG MKTAG('p', 'Q', 'G', 'T')    /* TGQ i-frame (appears in .UV files) */
#define pIQT_TAG MKTAG('p', 'I', 'Q', 'T')    /* TQI/UV2 i-frame (.UV2/.WVE) */
#define MVhd_TAG MKTAG('M', 'V', 'h', 'd')
#define MV0K_TAG MKTAG('M', 'V', '0', 'K')
#define MV0F_TAG MKTAG('M', 'V', '0', 'F')
#define MVIh_TAG MKTAG('M', 'V', 'I', 'h')    /* CMV header */
#define MVIf_TAG MKTAG('M', 'V', 'I', 'f')    /* CMV i-frame */

#if !defined(bswap_32)
static DWORD bswap_32(DWORD x)
{
	x= ((x<<8)&0xFF00FF00) | ((x>>8)&0x00FF00FF);
	x= (x>>16) | (x<<16);
	return x;
}
#endif
