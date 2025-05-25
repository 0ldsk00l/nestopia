/*
Copyright (c) 2012-2025 R. Danbrook
Copyright (c) 2020-2025 Rupert Carmichael
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "hasher.h"

#include "crc32.h"
#include "md5.h"

uint32_t Hasher::crc(void *data, size_t size) {
    return crc32(0, data, size);
}

std::string Hasher::md5(void *data, size_t size) {
    MD5_CTX c;
    size_t md5len = size;
    uint8_t *dataptr = static_cast<uint8_t*>(data);
    uint8_t digest[16];
    char md5buf[33];

    MD5_Init(&c);
    MD5_Update(&c, dataptr, md5len);
    MD5_Final(digest, &c);

    auto nyb_hexchar = [](uint8_t nyb) -> char {
        nyb &= 0xf; // Lower nybble only
        if (nyb >= 10)
            nyb += 'a' - 10;
        else
            nyb += '0';
        return (char)nyb;
    };

    for (int i = 0; i < 16; ++i) {
        md5buf[i * 2] = nyb_hexchar(digest[i] >> 4);
        md5buf[(i * 2) + 1] =nyb_hexchar(digest[i]);
    }

    md5buf[32] = '\0';
    return std::string{md5buf};
}
