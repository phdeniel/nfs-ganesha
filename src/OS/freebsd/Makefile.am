noinst_LTLIBRARIES            = libgos.la

libgos_la_SOURCES = freebsd_mntent.c freebsd_dirent.c atsyscalls.c

bin_PROGRAMS               = testat_freebsd testdir_freebsd

testat_freebsd_SOURCES = testat.c

testat_freebsd_LDADD = -lgos

testdir_freebsd_SOURCES = testdir.c

testdir_freebsd_LDADD = -lgos

