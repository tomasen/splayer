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


#include "complex.h"

Complex polar_to_complex(const double r, const double theta){
    Complex result;
    result.re = r*cos(theta);
	result.im = r*sin(theta);
    return result;
}
Complex add_complex(const Complex a, const Complex b){
    Complex result;
    result.re = a.re + b.re;
    result.im = a.im + b.im;
    return result;
}
Complex sub_complex(const Complex a, const Complex b){
	Complex result;
	result.re = a.re - b.re;
	result.im = a.im - b.im;
	return result;
}
Complex mult_complex(const Complex a, const Complex b){
	Complex result;
	result.re = (a.re*b.re) - (a.im*b.im);
    	result.im = (a.re*b.im) + (a.im*b.re);
	return result;
}
double complex_abs(const Complex a){
  return sqrt(a.re*a.re + a.im*a.im);
}

