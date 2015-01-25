#AM_CPPFLAGS = -I$(top_builddir)/vendor
#
#HIREDIS_LIB = $(top_builddir)/vendor/hiredis/libhiredis.a

lib_LTLIBRARIES = libyi_rediskvstore.la

libyi_rediskvstore_la_SOURCES=RedisKVStore.h \
							  RedisKVStore.cc \
							  async.h \
							  async.c \
							  hiredis.h \
							  hiredis.c \
							  net.h \
							  net.c \
							  read.h \
							  read.c \
							  sds.h \
							  sds.c

noinst_PROGRAMS = example

example_SOURCES = example.cc
example_LDADD = libyi_rediskvstore.la
