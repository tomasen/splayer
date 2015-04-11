/*
    Audio Scout - audio content indexing software
    Copyright (C) 2010  D. Grant Starkweather & Evan Klinger
    
    Audio Scout is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    D. Grant Starkweather - dstarkweather@phash.org
    Evan Klinger          - eklinger@phash.org
*/

#include "fft.h"


void fft_calc(const int N, const double *x, Complex *X, Complex *P, const int step, const Complex *twids){
    int k;
    Complex *S = P + N/2;
    if (N == 1){
	X[0].re = x[0];
        X[0].im = 0;
	return;
    }
    
    fft_calc(N/2, x,      S,   X,2*step, twids);
    fft_calc(N/2, x+step, P,   X,2*step, twids);
 
    for (k=0;k<N/2;k++){
		P[k] = mult_complex(P[k],twids[k*step]);
		X[k]     = add_complex(S[k],P[k]);
		X[k+N/2] = sub_complex(S[k],P[k]);
    }

}


int fft(const double *x, const int N, Complex *X){

    Complex *twiddle_factors = (Complex*)malloc(sizeof(Complex)*(N/2));
    Complex *Xt = (Complex*)malloc(sizeof(Complex)*(N));
    int k;
    for (k=0;k<N/2;k++){
	twiddle_factors[k] = polar_to_complex(1.0, 2.0*PI*k/N);
    }
    fft_calc(N, x, X, Xt, 1, twiddle_factors);

    free(twiddle_factors);
    free(Xt);

    return 0;

}
