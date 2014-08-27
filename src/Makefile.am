bin_PROGRAMS = ripper6

ripper6_SOURCES = main.cpp

EXTRA_ripper6_SOURCES  = check_cdfm.cpp
EXTRA_ripper6_SOURCES += check_cmf.cpp
EXTRA_ripper6_SOURCES += check_midi.cpp
EXTRA_ripper6_SOURCES += check_riff.cpp
EXTRA_ripper6_SOURCES += check_s3m.cpp
EXTRA_ripper6_SOURCES += check_tbsa.cpp
EXTRA_ripper6_SOURCES += check_voc.cpp

WARNINGS = -Wall -Wextra -Wno-unused-parameter

AM_CPPFLAGS  = $(WARNINGS)
AM_CPPFLAGS += $(libgamecommon_CPPFLAGS)

AM_CXXFLAGS  = $(libgamecommon_CFLAGS)

AM_LDFLAGS  = $(libgamecommon_LIBS)
