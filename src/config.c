/*
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
 * Copyright (C) 2007  The OpenTyrian Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "opentyr.h"
#include "config.h"

#include <stdio.h>
#include <string.h>

const JE_byte StringCryptKey[10] = {99, 204, 129, 63, 255, 71, 19, 25, 62, 1};

void JE_decryptstring( JE_string s, JE_byte len )
{
    int i;

    for (i = len-1; i >= 0; i--)
    {
        s[i] ^= StringCryptKey[((i+1) % 10)];
        if (i > 0)
        {
            s[i] ^= s[i-1];
        }
    }
}

void JE_readcryptln( FILE* f, JE_string s )
{
    int size;

    size = getc(f);

    fread(s, size, 1, f);
/*    s[0] = (char)size;*/

    JE_decryptstring(s, size);

}

void JE_skipcryptln( FILE* f )
{
    char size;

    size = getc(f);

    fseek(f, size, SEEK_CUR);
}
