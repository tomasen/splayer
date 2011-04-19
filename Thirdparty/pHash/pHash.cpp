/*

    pHash, the open source perceptual hash library
    Copyright (C) 2009 Aetilius, Inc.
    All rights reserved.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Evan Klinger - eklinger@phash.org
    D Grant Starkweather - dstarkweather@phash.org

*/

#include "pHash.h"


const char phash_project[] = "%s. Copyright 2008-2010 Aetilius, Inc.";
char phash_version[255] = {0};

/*const char* ph_about(){
	if(phash_version[0] != 0)
 		return phash_version;

 	snprintf(phash_version, sizeof(phash_version), phash_project, PACKAGE_STRING);
 	return phash_version;
}
*/
#define max(a,b) (((a)>(b))?(a):(b))

static bool keepStats = false;

struct ph_stats
{
	ulong64 reads;
	ulong64 avg_atime, height;

};

enum ph_option
{
	PH_STATS
};

void ph_set_option(ph_option opt, int val)
{
	switch(opt)
	{
		case PH_STATS:
			keepStats = (bool)val;
			break;
		default:
			break;
	}
}
