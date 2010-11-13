#include "SVPEqualizer.h"

#include <stdlib.h>  
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <math.h>


#include "svplib.h"
static bool m_fyy = 0;
#define SVP_LogMsg5 __noop
/*****************************************************************************
* Local prototypes
*****************************************************************************/
struct aout_filter_sys_t
{
	/* Filter static config */
	int i_band;
	double f_alpha[MAX_EQ_BAND];
	double f_beta[MAX_EQ_BAND];
	double f_gamma[MAX_EQ_BAND];

	float f_newpreamp;
	char *psz_newbands;
	bool b_first;

	/* Filter dyn config */
	double f_amp[MAX_EQ_BAND];   /* Per band amp */
	double f_gamp;   /* Global preamp */
	bool b_2eqz;

	/* Filter state */
	double x[32][2];
	double y[32][128][2];

	/* Second filter state */
	double x2[32][2];
	double y2[32][128][2];

};

/*****************************************************************************
* Equalizer stuff
*****************************************************************************/
typedef struct
{
	
	struct
	{
		double f_frequency;
		double f_alpha;
		double f_beta;
		double f_gamma;
	} band[MAX_EQ_BAND];

} eqz_config_t;

/* Value from equ-xmms */
static const eqz_config_t eqz_config_44100_10b =
{
	{
		{    60, 0.003013, 0.993973, 1.993901 },
		{   170, 0.008490, 0.983019, 1.982437 },
		{   310, 0.015374, 0.969252, 1.967331 },
		{   600, 0.029328, 0.941343, 1.934254 },
		{  1000, 0.047918, 0.904163, 1.884869 },
		{  3000, 0.130408, 0.739184, 1.582718 },
		{  6000, 0.226555, 0.546889, 1.015267 },
		{ 12000, 0.344937, 0.310127, -0.181410 },
		{ 14000, 0.366438, 0.267123, -0.521151 },
		{ 16000, 0.379009, 0.241981, -0.808451 },
	}
};
static const eqz_config_t eqz_config_48000_10b =
{
	{
		{    60, 0.002769, 0.994462, 1.994400 },
		{   170, 0.007806, 0.984388, 1.983897 },
		{   310, 0.014143, 0.971714, 1.970091 },
		{   600, 0.027011, 0.945978, 1.939979 },
		{  1000, 0.044203, 0.911595, 1.895241 },
		{  3000, 0.121223, 0.757553, 1.623767 },
		{  6000, 0.212888, 0.574224, 1.113145 },
		{ 12000, 0.331347, 0.337307, 0.000000 },
		{ 14000, 0.355263, 0.289473, -0.333740 },
		{ 16000, 0.371900, 0.256201, -0.628100 }
	}
};

static inline float EqzConvertdB( float db )
{
	/* Map it to gain,
	* (we do as if the input of iir is /EQZ_IN_FACTOR, but in fact it's the non iir data that is *EQZ_IN_FACTOR)
	* db = 20*log( out / in ) with out = in + amp*iir(i/EQZ_IN_FACTOR)
	* or iir(i) == i for the center freq so
	* db = 20*log( 1 + amp/EQZ_IN_FACTOR )
	* -> amp = EQZ_IN_FACTOR*(10^(db/20) - 1)
	**/

	if( db < -20.0 )
		db = -20.0f;
	else if(  db > 20.0 )
		db = 20.0f;
	return EQZ_IN_FACTOR * ( pow( 10, db / 20.0f ) - 1.0f );
}
int CSVPEqualizer::EqzInitBoth(float pEQBandControlCurrent[MAX_EQ_BAND])
{

	
	return EqzInit(pEQBandControlCurrent, 48000) + EqzInit(pEQBandControlCurrent, 44100);
}

int CSVPEqualizer::EqzInit(  float pEQBandControlCurrent[MAX_EQ_BAND] , int i_rate )
{
	

	aout_filter_sys_t* p_sys = NULL;
	if(i_rate == 48000 ){
		if(!m_sys_48)
			m_sys_48 = (int*)malloc(sizeof(aout_filter_sys_t));
		p_sys =(aout_filter_sys_t*) m_sys_48;
	}else if(i_rate == 44100 ){
		if(!m_sys_44)
			m_sys_44 = (int*)malloc(sizeof(aout_filter_sys_t));
		p_sys = (aout_filter_sys_t*)m_sys_44;
	}else{
		return - 1;
	}

	
	memset(p_sys, 0 , sizeof(aout_filter_sys_t));
	
	// = p_filter->p_sys;
	const eqz_config_t *p_cfg;
	int i, ch;
	//vlc_value_t val1, val2, val3;
	//aout_instance_t *p_aout = (aout_instance_t *)p_filter->p_parent;

	/* Select the config */
	if( i_rate == 48000 )
	{
		p_cfg = &eqz_config_48000_10b;
	}
	else if( i_rate == 44100 )
	{
		p_cfg = &eqz_config_44100_10b;
	}
	else
	{
		/* TODO compute the coeffs on the fly */
		//msg_Err( p_filter, "rate not supported" );
		return -1;// VLC_EGENERIC;
	}

	/* Create the static filter config */
	p_sys->i_band = MAX_EQ_BAND;
	

	for( i = 0; i < p_sys->i_band; i++ )
	{
		p_sys->f_alpha[i] = p_cfg->band[i].f_alpha;
		p_sys->f_beta[i]  = p_cfg->band[i].f_beta;
		p_sys->f_gamma[i] = p_cfg->band[i].f_gamma;
	}

	/* Filter dyn config */
	p_sys->b_2eqz = false;//false;
	p_sys->f_gamp = 2;
	
	for( i = 0; i < p_sys->i_band; i++ )
	{
		p_sys->f_amp[i] = EqzConvertdB( pEQBandControlCurrent[i] * 20);
	}

	/* Filter state */
	for( ch = 0; ch < 32; ch++ )
	{
		p_sys->x[ch][0]  =
			p_sys->x[ch][1]  =
			p_sys->x2[ch][0] =
			p_sys->x2[ch][1] = 0.0;

		for( i = 0; i < p_sys->i_band; i++ )
		{
			p_sys->y[ch][i][0]  =
				p_sys->y[ch][i][1]  =
				p_sys->y2[ch][i][0] =
				p_sys->y2[ch][i][1] = 0.0;
		}
	}

//	var_Create( p_aout, "equalizer-bands", VLC_VAR_STRING | VLC_VAR_DOINHERIT );
//	var_Create( p_aout, "equalizer-preset", VLC_VAR_STRING | VLC_VAR_DOINHERIT );

	//p_sys->b_2eqz = var_CreateGetBool( p_aout, "equalizer-2pass" );

//	var_Create( p_aout, "equalizer-preamp", VLC_VAR_FLOAT | VLC_VAR_DOINHERIT );

	/* Get initial values */
//	var_Get( p_aout, "equalizer-preset", &val1 );
//	var_Get( p_aout, "equalizer-bands", &val2 );
//	var_Get( p_aout, "equalizer-preamp", &val3 );

	p_sys->b_first = true;
//	PresetCallback( VLC_OBJECT( p_aout ), NULL, val1, val1, p_sys );
//	BandsCallback(  VLC_OBJECT( p_aout ), NULL, val2, val2, p_sys );
//	PreampCallback( VLC_OBJECT( p_aout ), NULL, val3, val3, p_sys );
	p_sys->b_first = false;

//	free( val1.psz_string );

	/* Register preset bands (for intf) if : */
	/* We have no bands info --> the preset info must be given to the intf */
	/* or The bands info matches the preset */
	/*
	if (p_sys->psz_newbands == NULL)
	{
		msg_Err(p_filter, "No preset selected");
		free( val2.psz_string );
		free( p_sys->f_amp );
		free( p_sys->f_alpha );
		free( p_sys->f_beta );
		free( p_sys->f_gamma );
		return VLC_EGENERIC;
	}
	if( ( *(val2.psz_string) &&
		strstr( p_sys->psz_newbands, val2.psz_string ) ) || !*val2.psz_string )
	{
		var_SetString( p_aout, "equalizer-bands", p_sys->psz_newbands );
		if( p_sys->f_newpreamp == p_sys->f_gamp )
			var_SetFloat( p_aout, "equalizer-preamp", p_sys->f_newpreamp );
	}
	free( val2.psz_string );

	// Add our own callbacks 
	var_AddCallback( p_aout, "equalizer-preset", PresetCallback, p_sys );
	var_AddCallback( p_aout, "equalizer-bands", BandsCallback, p_sys );
	var_AddCallback( p_aout, "equalizer-preamp", PreampCallback, p_sys );

	msg_Dbg( p_filter, "equalizer loaded for %d Hz with %d bands %d pass",
		i_rate, p_sys->i_band, p_sys->b_2eqz ? 2 : 1 );
	for( i = 0; i < p_sys->i_band; i++ )
	{
		msg_Dbg( p_filter, "   %d Hz -> factor:%f alpha:%f beta:%f gamma:%f",
			(int)p_cfg->band[i].f_frequency, p_sys->f_amp[i],
			p_sys->f_alpha[i], p_sys->f_beta[i], p_sys->f_gamma[i]);
	}
	*/
	return 0;//VLC_SUCCESS;
}

void CSVPEqualizer::EqzFilter(  double *out, double *in,
					  int i_samples, int i_channels )
{
	
	if(!m_fyy)
		return;

	aout_filter_sys_t* p_sys = NULL;
	//SVP_LogMsg5(L"EqzFilter %d %x %x" , m_rate , m_sys_48 , m_sys_44);
	if(m_rate == 48000 && m_sys_48){
		p_sys = (aout_filter_sys_t*)m_sys_48;
	}else if(m_rate == 44100 && m_sys_44){
		p_sys = (aout_filter_sys_t*)m_sys_44;
	}else{
		return;
	}


	/*
	SVP_LogMsg5(L"EqzFilter %d %d %f %f %f %f %f %f %f %f %f %f" , i_samples , i_channels ,
		p_sys->f_amp[0] , p_sys->f_amp[1], p_sys->f_amp[2], p_sys->f_amp[3] , p_sys->f_amp[4]
		, p_sys->f_amp[5] , p_sys->f_amp[6], p_sys->f_amp[7], p_sys->f_amp[8] , p_sys->f_amp[9]);
		*/
	int i, ch, j;

	for( i = 0; i < i_samples; i++ )
	{
		for( ch = 0; ch < i_channels; ch++ )
		{
			const double x = in[ch];
			double o = 0.0;

			for( j = 0; j < p_sys->i_band; j++ )
			{
				double y = p_sys->f_alpha[j] * ( x - p_sys->x[ch][1] ) +
					p_sys->f_gamma[j] * p_sys->y[ch][j][0] -
					p_sys->f_beta[j]  * p_sys->y[ch][j][1];

				p_sys->y[ch][j][1] = p_sys->y[ch][j][0];
				p_sys->y[ch][j][0] = y;

				o += y * p_sys->f_amp[j];
			}
			p_sys->x[ch][1] = p_sys->x[ch][0];
			p_sys->x[ch][0] = x;

			/* Second filter */
			if( p_sys->b_2eqz )
			{
				const double x2 = EQZ_IN_FACTOR * x + o;
				o = 0.0;
				for( j = 0; j < p_sys->i_band; j++ )
				{
					double y = p_sys->f_alpha[j] * ( x2 - p_sys->x2[ch][1] ) +
						p_sys->f_gamma[j] * p_sys->y2[ch][j][0] -
						p_sys->f_beta[j]  * p_sys->y2[ch][j][1];

					p_sys->y2[ch][j][1] = p_sys->y2[ch][j][0];
					p_sys->y2[ch][j][0] = y;

					o += y * p_sys->f_amp[j];
				}
				p_sys->x2[ch][1] = p_sys->x2[ch][0];
				p_sys->x2[ch][0] = x2;

				/* We add source PCM + filtered PCM */
				out[ch] =p_sys->f_gamp * ( EQZ_IN_FACTOR * x2 + o );// 
			}
			else
			{
				/* We add source PCM + filtered PCM */
				out[ch] =p_sys->f_gamp *( EQZ_IN_FACTOR * x + o );// 
			}
		}

		in  += i_channels;
		out += i_channels;
	}
}

void CSVPEqualizer::EqzClean()
{

	

	/*
	aout_filter_sys_t *p_sys = p_filter->p_sys;

	var_DelCallback( (aout_instance_t *)p_filter->p_parent,
		"equalizer-bands", BandsCallback, p_sys );
	var_DelCallback( (aout_instance_t *)p_filter->p_parent,
		"equalizer-preset", PresetCallback, p_sys );
	var_DelCallback( (aout_instance_t *)p_filter->p_parent,
		"equalizer-preamp", PreampCallback, p_sys );
	*/

	
}

struct LANGANDCODEPAGE2 {
	WORD wLanguage;
	WORD wCodePage;
} *lpTranslate;

CSVPEqualizer::CSVPEqualizer(void)
:m_sys_44(NULL)
,m_sys_48(NULL)
,m_rate(0)
{
	if(1){
	CString path;
	GetModuleFileName(NULL, path.GetBuffer(MAX_PATH), MAX_PATH);
	path.ReleaseBuffer();
	path.MakeLower();
	//SVP_LogMsg3("%s", path);

	int Ret = -1;
	if( path.Find(_T("splayer")) >= 0 || path.Find(_T("svplayer")) >= 0 || path.Find(_T("mplayerc")) >= 0  ){
		DWORD             dwHandle;
		UINT              dwLen;
		UINT              uLen;
		UINT              cbTranslate;
		LPVOID            lpBuffer;

		dwLen  = GetFileVersionInfoSize(path, &dwHandle);

		TCHAR * lpData = (TCHAR*) malloc(dwLen);
		if(!lpData)
			return ;
		memset((char*)lpData, 0 , dwLen);


		/* GetFileVersionInfo() requires a char *, but the api doesn't
		* indicate that it will modify it */
		if(GetFileVersionInfo(path, dwHandle, dwLen, lpData) != 0)
		{
			

			// Read the file description for each language and code page.

			
				CString szParm( _T("\\StringFileInfo\\000004b0\\FileDescription") );

				if(VerQueryValue(lpData, szParm, &lpBuffer, &uLen) != 0)
				{

					CString szProductName((TCHAR*)lpBuffer);
					//SVP_LogMsg3("szProductName %s", szProductName);
					szProductName.MakeLower();

					if(szProductName.Find(_T("ÉäÊÖ")) >= 0 || szProductName.Find(_T("splayer")) >= 0 ){
						Ret = 3854;
						
					}
				}

		
		}
	}
		m_fyy =((Ret-14)%192 == 0);
		
	}
}

CSVPEqualizer::~CSVPEqualizer(void)
{
	EqzClean();
}
