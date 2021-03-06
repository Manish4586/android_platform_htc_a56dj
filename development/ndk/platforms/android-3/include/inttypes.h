
/*
 * Copyright (c) 1997, 2005 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef	_INTTYPES_H_
#define	_INTTYPES_H_

#include <stdint.h>
#include <sys/cdefs.h>

#if !defined(__cplusplus) || defined(__STDC_FORMAT_MACROS)

#define	PRId8			"d"		
#define	PRId16			"d"		
#define	PRId32			"d"		
#define	PRId64			"lld"		

#define	PRIdLEAST8		"d"		
#define	PRIdLEAST16		"d"		
#define	PRIdLEAST32		"d"		
#define	PRIdLEAST64		"lld"		

#define	PRIdFAST8		"d"		
#define	PRIdFAST16		"d"		
#define	PRIdFAST32		"d"		
#define	PRIdFAST64		"lld"		

#define	PRIdMAX			"jd"		
#define	PRIdPTR			"d"		

#define	PRIi8			"i"		
#define	PRIi16			"i"		
#define	PRIi32			"i"		
#define	PRIi64			"lli"		

#define	PRIiLEAST8		"i"		
#define	PRIiLEAST16		"i"		
#define	PRIiLEAST32		"i"		
#define	PRIiLEAST64		"lli"		

#define	PRIiFAST8		"i"		
#define	PRIiFAST16		"i"		
#define	PRIiFAST32		"i"		
#define	PRIiFAST64		"lli"		

#define	PRIiMAX			"ji"		
#define	PRIiPTR			"i"		

#define	PRIo8			"o"		
#define	PRIo16			"o"		
#define	PRIo32			"o"		
#define	PRIo64			"llo"		

#define	PRIoLEAST8		"o"		
#define	PRIoLEAST16		"o"		
#define	PRIoLEAST32		"o"		
#define	PRIoLEAST64		"llo"		

#define	PRIoFAST8		"o"		
#define	PRIoFAST16		"o"		
#define	PRIoFAST32		"o"		
#define	PRIoFAST64		"llo"		

#define	PRIoMAX			"jo"		
#define	PRIoPTR			"o"		

#define	PRIu8			"u"		
#define	PRIu16			"u"		
#define	PRIu32			"u"		
#define	PRIu64			"llu"		

#define	PRIuLEAST8		"u"		
#define	PRIuLEAST16		"u"		
#define	PRIuLEAST32		"u"		
#define	PRIuLEAST64		"llu"		

#define	PRIuFAST8		"u"		
#define	PRIuFAST16		"u"		
#define	PRIuFAST32		"u"		
#define	PRIuFAST64		"llu"		

#define	PRIuMAX			"ju"		
#define	PRIuPTR			"u"		

#define	PRIx8			"x"		
#define	PRIx16			"x"		
#define	PRIx32			"x"		
#define	PRIx64			"llx"		

#define	PRIxLEAST8		"x"		
#define	PRIxLEAST16		"x"		
#define	PRIxLEAST32		"x"		
#define	PRIxLEAST64		"llx"		

#define	PRIxFAST8		"x"		
#define	PRIxFAST16		"x"		
#define	PRIxFAST32		"x"		
#define	PRIxFAST64		"llx"		

#define	PRIxMAX			"jx"		
#define	PRIxPTR			"x"		

#define	PRIX8			"X"		
#define	PRIX16			"X"		
#define	PRIX32			"X"		
#define	PRIX64			"llX"		

#define	PRIXLEAST8		"X"		
#define	PRIXLEAST16		"X"		
#define	PRIXLEAST32		"X"		
#define	PRIXLEAST64		"llX"		

#define	PRIXFAST8		"X"		
#define	PRIXFAST16		"X"		
#define	PRIXFAST32		"X"		
#define	PRIXFAST64		"llX"		

#define	PRIXMAX			"jX"		
#define	PRIXPTR			"X"		

#define	SCNd8			"hhd"		
#define	SCNd16			"hd"		
#define	SCNd32			"d"		
#define	SCNd64			"lld"		

#define	SCNdLEAST8		"hhd"		
#define	SCNdLEAST16		"hd"		
#define	SCNdLEAST32		"d"		
#define	SCNdLEAST64		"lld"		

#define	SCNdFAST8		"hhd"		
#define	SCNdFAST16		"hd"		
#define	SCNdFAST32		"d"		
#define	SCNdFAST64		"lld"		

#define	SCNdMAX			"jd"		
#define	SCNdPTR			"d"		

#define	SCNi8			"hhi"		
#define	SCNi16			"hi"		
#define	SCNi32			"i"		
#define	SCNi64			"lli"		

#define	SCNiLEAST8		"hhi"		
#define	SCNiLEAST16		"hi"		
#define	SCNiLEAST32		"i"		
#define	SCNiLEAST64		"lli"		

#define	SCNiFAST8		"hhi"		
#define	SCNiFAST16		"hi"		
#define	SCNiFAST32		"i"		
#define	SCNiFAST64		"lli"		

#define	SCNiMAX			"ji"		
#define	SCNiPTR			"i"		

#define	SCNo8			"hho"		
#define	SCNo16			"ho"		
#define	SCNo32			"o"		
#define	SCNo64			"llo"		

#define	SCNoLEAST8		"hho"		
#define	SCNoLEAST16		"ho"		
#define	SCNoLEAST32		"o"		
#define	SCNoLEAST64		"llo"		

#define	SCNoFAST8		"hho"		
#define	SCNoFAST16		"ho"		
#define	SCNoFAST32		"o"		
#define	SCNoFAST64		"llo"		

#define	SCNoMAX			"jo"		
#define	SCNoPTR			"o"		

#define	SCNu8			"hhu"		
#define	SCNu16			"hu"		
#define	SCNu32			"u"		
#define	SCNu64			"llu"		

#define	SCNuLEAST8		"hhu"		
#define	SCNuLEAST16		"hu"		
#define	SCNuLEAST32		"u"		
#define	SCNuLEAST64		"llu"		

#define	SCNuFAST8		"hhu"		
#define	SCNuFAST16		"hu"		
#define	SCNuFAST32		"u"		
#define	SCNuFAST64		"llu"		

#define	SCNuMAX			"ju"		
#define	SCNuPTR			"u"		

#define	SCNx8			"hhx"		
#define	SCNx16			"hx"		
#define	SCNx32			"x"		
#define	SCNx64			"llx"		

#define	SCNxLEAST8		"hhx"		
#define	SCNxLEAST16		"hx"		
#define	SCNxLEAST32		"x"		
#define	SCNxLEAST64		"llx"		

#define	SCNxFAST8		"hhx"		
#define	SCNxFAST16		"hx"		
#define	SCNxFAST32		"x"		
#define	SCNxFAST64		"llx"		

#define	SCNxMAX			"jx"		
#define	SCNxPTR			"x"		

#endif 

typedef struct {
	intmax_t quot;		
	intmax_t rem;		
} imaxdiv_t;

__BEGIN_DECLS
intmax_t	strtoimax(const char *, char **, int);
uintmax_t	strtoumax(const char *, char **, int);

intmax_t	strntoimax(const char *nptr, char **endptr, int base, size_t n);
uintmax_t	strntoumax(const char *nptr, char **endptr, int base, size_t n);
__END_DECLS

#endif 
