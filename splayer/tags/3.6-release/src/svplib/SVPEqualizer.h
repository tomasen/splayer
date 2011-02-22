#pragma once

#define MAX_EQ_BAND 10
#define EQZ_IN_FACTOR (0.25f)


class CSVPEqualizer
{
	int EqzInit(float pEQBandControlCurrent[MAX_EQ_BAND] , int i_rate = 0 );

public:
	CSVPEqualizer(void);
	~CSVPEqualizer(void);

	int* m_sys_44;
	int* m_sys_48;
	int m_rate;

	void EqzFilter(  double *out, double *in,
		int i_samples, int i_channels );
	int EqzInitBoth(float pEQBandControlCurrent[MAX_EQ_BAND]);
	void EqzClean( );
};
