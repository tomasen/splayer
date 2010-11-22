# Copyright (C) 1999 Scott Thomas Haug <scott@id3.org>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

LT_VERSION = 1:0:0

INCLUDES = -I$(top_srcdir)/include

zlib_sources = \
  adler32.c  \
  compress.c \
  crc32.c    \
  deflate.c  \
  gzio.c     \
  infblock.c \
  infcodes.c \
  inffast.c  \
  inflate.c  \
  inftrees.c \
  infutil.c  \
  trees.c    \
  uncompr.c  \
  zutil.c

lib_LTLIBRARIES = libz.la
libz_la_SOURCES = $(zlib_sources)

libz_la_LDFLAGS = -version-info $(LT_VERSION)
