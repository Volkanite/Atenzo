/*
This code was adapted from EcomCat by Charlie Miller and Chris Valasek.
It is based on the file /ecom/content/code/ecomcat_api/security.py
which is written in Python. This is a pure C implementation of that file.
The only modification done was changing the 5-byte "shared secret"
to MazdA which works for 1st gen Mazda Atenzas.
*/

#include <stdlib.h>


int GetKeyFromSeed( char* Seed )
{
    int v8,v9,v10;
    int key;
    char s1,s2,s3,s4,s5;
    
    //"MazdA" in ASCII [4d 61 7a 64 41]
    s1 = 0x4d;
    s2 = 0x61;
    s3 = 0x7a;
    s4 = 0x64;
    s5 = 0x41;
    
    //convert seed to int
    int seed_int = strtol(Seed, NULL, 16);
    
    int or_ed_seed = ((seed_int & 0xFF0000) >> 16) | (seed_int & 0xFF00) | (s1 << 24) | (seed_int & 0xff) << 16;
    
    int mucked_value = 0xc541a9;
    
    for (int i = 0; i < 32; i++) 
    {
        int a_bit = ((or_ed_seed >> i) & 1 ^ mucked_value & 1) << 23;
        int v9 = v10 = v8 = a_bit | (mucked_value >> 1);
        mucked_value = v10 & 0xEF6FD7 | ((((v9 & 0x100000) >> 20) ^ ((v8 & 0x800000) >> 23)) << 20) | (((((mucked_value >> 1) & 0x8000) >> 15) ^ ((v8 & 0x800000) >> 23)) << 15) | (((((mucked_value >> 1) & 0x1000) >> 12) ^ ((v8 & 0x800000) >> 23)) << 12) | 32 * ((((mucked_value >> 1) & 0x20) >> 5) ^ ((v8 & 0x800000) >> 23)) | 8 * ((((mucked_value >> 1) & 8) >> 3) ^ ((v8 & 0x800000) >> 23));
    }
    
    for (int j = 0; j < 32; j++) 
    {
        int v11 = ((((s5 << 24) | (s4 << 16) | s2 | (s3 << 8)) >> j) & 1 ^ mucked_value & 1) << 23;
        int v12 = v11 | (mucked_value >> 1);
        int v13 = v11 | (mucked_value >> 1);
        int v14 = v11 | (mucked_value >> 1);
        mucked_value = v14 & 0xEF6FD7 | ((((v13 & 0x100000) >> 20) ^ ((v12 & 0x800000) >> 23)) << 20) | (((((mucked_value >> 1) & 0x8000) >> 15) ^ ((v12 & 0x800000) >> 23)) << 15) | (((((mucked_value >> 1) & 0x1000) >> 12) ^ ((v12 & 0x800000) >> 23)) << 12) | 32 * ((((mucked_value >> 1) & 0x20) >> 5) ^ ((v12 & 0x800000) >> 23)) | 8 * ((((mucked_value >> 1) & 8) >> 3) ^ ((v12 & 0x800000) >> 23));
    }
    
    key = ((mucked_value & 0xF0000) >> 16) | (16 * (mucked_value & 0xF)) | ((((mucked_value & 0xF00000) >> 20) | ((mucked_value & 0xF000) >> 8)) << 8) | (((mucked_value & 0xFF0) >> 4) << 16);
    
    return key;
}
