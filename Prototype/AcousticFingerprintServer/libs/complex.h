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

#include <stdlib.h>
#include <math.h>

#ifndef COMPLEX_H
#define COMPLEX_H

typedef struct complex {
     double re;
     double im;
} Complex;

Complex polar_to_complex(const double r, const double theta);

Complex add_complex(const Complex a, const Complex b);

Complex sub_complex(const Complex a, const Complex b);

Complex mult_complex(const Complex a, const Complex b);

double complex_abs(const Complex a);
#endif
