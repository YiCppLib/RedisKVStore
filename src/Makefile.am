AM_CPPFLAGS = -I$(top_builddir)/vendor

HIREDIS_LIB = $(top_builddir)/vendor/hiredis/libhiredis.a

lib_LIBRARIES = libyi_rediskvstore.a

libyi_rediskvstore_a_SOURCES=RedisKVStore.h \
							 RedisKVStore.cc

noinst_PROGRAMS = example

example_SOURCES = example.cc
example_LDADD = libyi_rediskvstore.a $(HIREDIS_LIB)
