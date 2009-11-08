#pragma once

#define MAX_EQ_BAND 10
#define EQZ_IN_FACTOR (0.25)

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

class CSVPEqualizer
{
	int EqzInit(float pEQBandControlCurrent[MAX_EQ_BAND] , int i_rate = 0 );

public:
	CSVPEqualizer(void);
	~CSVPEqualizer(void);

	aout_filter_sys_t* m_sys_44;
	aout_filter_sys_t* m_sys_48;
	int m_rate;

	void EqzFilter(  double *out, double *in,
		int i_samples, int i_channels );
	int EqzInitBoth(float pEQBandControlCurrent[MAX_EQ_BAND]);
	void EqzClean( );
};
