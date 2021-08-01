/**
 * @file diyfp.c
 * @author 
 * @date 2021/07/02
 * @brief The complementation of diy_fp.
 *
 * Copyright (C) 2021 FMSoft <https://www.fmsoft.cn>
 *
 * This file is a part of PurC (short for Purring Cat), an HVML interpreter.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * This implementation of HTML parser is derived from Lexbor <http://lexbor.com/>.
 *
 * Copyright (C) 2018-2020 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include "html/core/diyfp.h"


typedef struct {
  uint64_t significand;
  int16_t  bin_exp;
  int16_t  dec_exp;
}
pchtml_diyfp_cpe_t;


static const pchtml_diyfp_cpe_t pchtml_cached_powers[] = {
  { pchtml_uint64_hl(0xfa8fd5a0, 0x081c0288), -1220, -348 },
  { pchtml_uint64_hl(0xbaaee17f, 0xa23ebf76), -1193, -340 },
  { pchtml_uint64_hl(0x8b16fb20, 0x3055ac76), -1166, -332 },
  { pchtml_uint64_hl(0xcf42894a, 0x5dce35ea), -1140, -324 },
  { pchtml_uint64_hl(0x9a6bb0aa, 0x55653b2d), -1113, -316 },
  { pchtml_uint64_hl(0xe61acf03, 0x3d1a45df), -1087, -308 },
  { pchtml_uint64_hl(0xab70fe17, 0xc79ac6ca), -1060, -300 },
  { pchtml_uint64_hl(0xff77b1fc, 0xbebcdc4f), -1034, -292 },
  { pchtml_uint64_hl(0xbe5691ef, 0x416bd60c), -1007, -284 },
  { pchtml_uint64_hl(0x8dd01fad, 0x907ffc3c),  -980, -276 },
  { pchtml_uint64_hl(0xd3515c28, 0x31559a83),  -954, -268 },
  { pchtml_uint64_hl(0x9d71ac8f, 0xada6c9b5),  -927, -260 },
  { pchtml_uint64_hl(0xea9c2277, 0x23ee8bcb),  -901, -252 },
  { pchtml_uint64_hl(0xaecc4991, 0x4078536d),  -874, -244 },
  { pchtml_uint64_hl(0x823c1279, 0x5db6ce57),  -847, -236 },
  { pchtml_uint64_hl(0xc2109436, 0x4dfb5637),  -821, -228 },
  { pchtml_uint64_hl(0x9096ea6f, 0x3848984f),  -794, -220 },
  { pchtml_uint64_hl(0xd77485cb, 0x25823ac7),  -768, -212 },
  { pchtml_uint64_hl(0xa086cfcd, 0x97bf97f4),  -741, -204 },
  { pchtml_uint64_hl(0xef340a98, 0x172aace5),  -715, -196 },
  { pchtml_uint64_hl(0xb23867fb, 0x2a35b28e),  -688, -188 },
  { pchtml_uint64_hl(0x84c8d4df, 0xd2c63f3b),  -661, -180 },
  { pchtml_uint64_hl(0xc5dd4427, 0x1ad3cdba),  -635, -172 },
  { pchtml_uint64_hl(0x936b9fce, 0xbb25c996),  -608, -164 },
  { pchtml_uint64_hl(0xdbac6c24, 0x7d62a584),  -582, -156 },
  { pchtml_uint64_hl(0xa3ab6658, 0x0d5fdaf6),  -555, -148 },
  { pchtml_uint64_hl(0xf3e2f893, 0xdec3f126),  -529, -140 },
  { pchtml_uint64_hl(0xb5b5ada8, 0xaaff80b8),  -502, -132 },
  { pchtml_uint64_hl(0x87625f05, 0x6c7c4a8b),  -475, -124 },
  { pchtml_uint64_hl(0xc9bcff60, 0x34c13053),  -449, -116 },
  { pchtml_uint64_hl(0x964e858c, 0x91ba2655),  -422, -108 },
  { pchtml_uint64_hl(0xdff97724, 0x70297ebd),  -396, -100 },
  { pchtml_uint64_hl(0xa6dfbd9f, 0xb8e5b88f),  -369,  -92 },
  { pchtml_uint64_hl(0xf8a95fcf, 0x88747d94),  -343,  -84 },
  { pchtml_uint64_hl(0xb9447093, 0x8fa89bcf),  -316,  -76 },
  { pchtml_uint64_hl(0x8a08f0f8, 0xbf0f156b),  -289,  -68 },
  { pchtml_uint64_hl(0xcdb02555, 0x653131b6),  -263,  -60 },
  { pchtml_uint64_hl(0x993fe2c6, 0xd07b7fac),  -236,  -52 },
  { pchtml_uint64_hl(0xe45c10c4, 0x2a2b3b06),  -210,  -44 },
  { pchtml_uint64_hl(0xaa242499, 0x697392d3),  -183,  -36 },
  { pchtml_uint64_hl(0xfd87b5f2, 0x8300ca0e),  -157,  -28 },
  { pchtml_uint64_hl(0xbce50864, 0x92111aeb),  -130,  -20 },
  { pchtml_uint64_hl(0x8cbccc09, 0x6f5088cc),  -103,  -12 },
  { pchtml_uint64_hl(0xd1b71758, 0xe219652c),   -77,   -4 },
  { pchtml_uint64_hl(0x9c400000, 0x00000000),   -50,    4 },
  { pchtml_uint64_hl(0xe8d4a510, 0x00000000),   -24,   12 },
  { pchtml_uint64_hl(0xad78ebc5, 0xac620000),     3,   20 },
  { pchtml_uint64_hl(0x813f3978, 0xf8940984),    30,   28 },
  { pchtml_uint64_hl(0xc097ce7b, 0xc90715b3),    56,   36 },
  { pchtml_uint64_hl(0x8f7e32ce, 0x7bea5c70),    83,   44 },
  { pchtml_uint64_hl(0xd5d238a4, 0xabe98068),   109,   52 },
  { pchtml_uint64_hl(0x9f4f2726, 0x179a2245),   136,   60 },
  { pchtml_uint64_hl(0xed63a231, 0xd4c4fb27),   162,   68 },
  { pchtml_uint64_hl(0xb0de6538, 0x8cc8ada8),   189,   76 },
  { pchtml_uint64_hl(0x83c7088e, 0x1aab65db),   216,   84 },
  { pchtml_uint64_hl(0xc45d1df9, 0x42711d9a),   242,   92 },
  { pchtml_uint64_hl(0x924d692c, 0xa61be758),   269,  100 },
  { pchtml_uint64_hl(0xda01ee64, 0x1a708dea),   295,  108 },
  { pchtml_uint64_hl(0xa26da399, 0x9aef774a),   322,  116 },
  { pchtml_uint64_hl(0xf209787b, 0xb47d6b85),   348,  124 },
  { pchtml_uint64_hl(0xb454e4a1, 0x79dd1877),   375,  132 },
  { pchtml_uint64_hl(0x865b8692, 0x5b9bc5c2),   402,  140 },
  { pchtml_uint64_hl(0xc83553c5, 0xc8965d3d),   428,  148 },
  { pchtml_uint64_hl(0x952ab45c, 0xfa97a0b3),   455,  156 },
  { pchtml_uint64_hl(0xde469fbd, 0x99a05fe3),   481,  164 },
  { pchtml_uint64_hl(0xa59bc234, 0xdb398c25),   508,  172 },
  { pchtml_uint64_hl(0xf6c69a72, 0xa3989f5c),   534,  180 },
  { pchtml_uint64_hl(0xb7dcbf53, 0x54e9bece),   561,  188 },
  { pchtml_uint64_hl(0x88fcf317, 0xf22241e2),   588,  196 },
  { pchtml_uint64_hl(0xcc20ce9b, 0xd35c78a5),   614,  204 },
  { pchtml_uint64_hl(0x98165af3, 0x7b2153df),   641,  212 },
  { pchtml_uint64_hl(0xe2a0b5dc, 0x971f303a),   667,  220 },
  { pchtml_uint64_hl(0xa8d9d153, 0x5ce3b396),   694,  228 },
  { pchtml_uint64_hl(0xfb9b7cd9, 0xa4a7443c),   720,  236 },
  { pchtml_uint64_hl(0xbb764c4c, 0xa7a44410),   747,  244 },
  { pchtml_uint64_hl(0x8bab8eef, 0xb6409c1a),   774,  252 },
  { pchtml_uint64_hl(0xd01fef10, 0xa657842c),   800,  260 },
  { pchtml_uint64_hl(0x9b10a4e5, 0xe9913129),   827,  268 },
  { pchtml_uint64_hl(0xe7109bfb, 0xa19c0c9d),   853,  276 },
  { pchtml_uint64_hl(0xac2820d9, 0x623bf429),   880,  284 },
  { pchtml_uint64_hl(0x80444b5e, 0x7aa7cf85),   907,  292 },
  { pchtml_uint64_hl(0xbf21e440, 0x03acdd2d),   933,  300 },
  { pchtml_uint64_hl(0x8e679c2f, 0x5e44ff8f),   960,  308 },
  { pchtml_uint64_hl(0xd433179d, 0x9c8cb841),   986,  316 },
  { pchtml_uint64_hl(0x9e19db92, 0xb4e31ba9),  1013,  324 },
  { pchtml_uint64_hl(0xeb96bf6e, 0xbadf77d9),  1039,  332 },
  { pchtml_uint64_hl(0xaf87023b, 0x9bf0ee6b),  1066,  340 },
};


#define PCHTML_DIYFP_D_1_LOG2_10 0.30102999566398114 /* 1 / log2(10). */


pchtml_diyfp_t
pchtml_cached_power_dec(int exp, int *dec_exp)
{
    unsigned int index;
    const pchtml_diyfp_cpe_t *cp;

    index = (exp + PCHTML_DECIMAL_EXPONENT_OFF) / PCHTML_DECIMAL_EXPONENT_DIST;
    cp = &pchtml_cached_powers[index];

    *dec_exp = cp->dec_exp;

    return pchtml_diyfp(cp->significand, cp->bin_exp);
}

pchtml_diyfp_t
pchtml_cached_power_bin(int exp, int *dec_exp)
{
    int k;
    unsigned int index;
    const pchtml_diyfp_cpe_t *cp;

    k = (int) ceil((-61 - exp) * PCHTML_DIYFP_D_1_LOG2_10)
        + PCHTML_DECIMAL_EXPONENT_OFF - 1;

    index = (unsigned) (k >> 3) + 1;

    cp = &pchtml_cached_powers[index];

    *dec_exp = -(PCHTML_DECIMAL_EXPONENT_MIN + (int) (index << 3));

    return pchtml_diyfp(cp->significand, cp->bin_exp);
}


#undef PCHTML_DIYFP_D_1_LOG2_10
