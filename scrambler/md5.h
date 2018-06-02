
/* MD5DEEP - md5.h
 *
 * By Jesse Kornblum
 *
 * This is a work of the US Government. In accordance with 17 USC 105,
 * copyright protection is not available for any work of the US Government.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/* $Id: md5.h 501 2011-09-25 17:12:27Z xchatty $ */

#ifndef __MD5_H
#define __MD5_H

typedef unsigned int uint32_t; 
typedef int int32_t; 

#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86))
#define _W64 __w64
#else
#define _W64
#endif
#endif

#ifndef _SIZE_T_DEFINED
#ifdef _WIN64
typedef unsigned __int64    size_t;
#else  /* _WIN64 */
typedef _W64 unsigned int   size_t;
#endif  /* _WIN64 */
#define _SIZE_T_DEFINED
#endif  /* _SIZE_T_DEFINED */

#ifndef __BEGIN_DECLS
#if defined(__cplusplus)
#define __BEGIN_DECLS   extern "C" {
#define __END_DECLS     }
#else
#define __BEGIN_DECLS
#define __END_DECLS
#endif
#endif

// -------------------------------------------------------------- 
// After this is the algorithm itself. You shouldn't change these

__BEGIN_DECLS

typedef struct {
  uint32_t buf[4];
  uint32_t bits[2];
  unsigned char in[64];
} context_md5_t;

// This is needed to make RSAREF happy on some MS-DOS compilers 
typedef context_md5_t MD5_CTX;

void MD5Init(context_md5_t *ctx);
void MD5Update(context_md5_t *context, const unsigned char *buf, size_t len);
void MD5Final(unsigned char digest[16], context_md5_t *context);
void MD5Transform(uint32_t buf[4], uint32_t const in[16]);

void hash_init_md5(void * ctx);
void hash_update_md5(void *ctx, const unsigned char *buf, size_t len);
void hash_final_md5(void *ctx, unsigned char *digest);

__END_DECLS

#endif /* ifndef __MD5_H */
