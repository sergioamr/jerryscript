/* Copyright 2016 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

/** Compare two strings. return an integer less than, equal to, or greater than zero
     if the first n character of s1 is found, respectively, to be less than, to match,
     or be greater than the first n character of s2.  */
int
strncmp (const char *s1, const char *s2, size_t n)
{
  while (n--)
  {
    int c1 = (unsigned char) *s1++;
    int c2 = (unsigned char) *s2++;
    int diff = c1 - c2;

    if (!c1 || diff)
    {
      return diff;
    }
  }

  return 0;
} /* strncmp */

/**
 * memcmp
 *
 * @return 0, if areas are equal;
 *         <0, if first area's content is lexicographically less, than second area's content;
 *         >0, otherwise
 */
int
memcmp (const void *s1, /**< first area */
        const void *s2, /**< second area */
        size_t n) /**< area size */
{
  const uint8_t *area1_p = (uint8_t *) s1, *area2_p = (uint8_t *) s2;
  while (n--)
  {
    int diff = ((int) *area1_p++) - ((int) *area2_p++);
    if (diff)
    {
      return diff;
    }
  }

  return 0;
} /* memcmp */

/**
 * State of pseudo-random number generator
 */
static uint32_t libc_random_gen_state[4] = { 1455997910, 1999515274, 1234451287, 1949149569 };

/**
 * Generate pseudo-random integer
 *
 * Note:
 *      The function implements George Marsaglia's XorShift random number generator
 *
 * @return integer in range [0; RAND_MAX]
 */
int
rand (void)
{
  uint32_t intermediate = libc_random_gen_state[0] ^ (libc_random_gen_state[0] << 11);
  intermediate ^= intermediate >> 8;

  libc_random_gen_state[0] = libc_random_gen_state[1];
  libc_random_gen_state[1] = libc_random_gen_state[2];
  libc_random_gen_state[2] = libc_random_gen_state[3];

  libc_random_gen_state[3] ^= libc_random_gen_state[3] >> 19;
  libc_random_gen_state[3] ^= intermediate;

  return libc_random_gen_state[3] % (RAND_MAX + 1u);
} /* rand */
