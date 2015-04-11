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

#ifndef _SERIALIZE_H
#define _SERIALIZE_H

#include <sys/param.h>

#ifdef __FreeBSD__
#include <machine/endian.h>
#warning "detected freeBSD machine"
#else
#include <endian.h>
#warning "detected gnu/linux machine"
#endif

#include <stdint.h>

#if CHAR_BIT != 8
#error "unsupported char size"
#endif

/* serialize functions */

float hosttonetf(float fval){
    float result;
    char *ptr_val = (char*)&fval;
    char *ptr_res = (char*)&result;
    ptr_res[0] = ptr_val[3];
    ptr_res[1] = ptr_val[2];
    ptr_res[2] = ptr_val[1];
    ptr_res[3] = ptr_val[0];
    return result;
}


#if __BYTE_ORDER == __BIG_ENDIAN
#warning "big endian detected"

#define hosttonet32(X) (((X & 0xff) << 24) +\
                        (((X>> 8) & 0xff) << 16) +\
                        (((X>>16) & 0xff) <<  8) +\
                         ((X>>24) & 0xff))

#define nettohost32 hosttonet32
#define nettohostf  hosttonetf


#elif __BYTE_ORDER == __LITTLE_ENDIAN
#warning "little endian detected"

#define hosttonet32(X)  X
#define hosttonetf(X)   X
#define nettohost32(X)  X
#define nettohostf(X)   X
#else
#error "endian unknown"
#endif /* BYTE_ORDER */ 



#endif /* _SERIALIZE_H */ 
