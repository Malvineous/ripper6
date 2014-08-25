bin_PROGRAMS = ripper6

ripper6_SOURCES = main.cpp

EXTRA_ripper6_SOURCES  = check_cdfm.cpp
EXTRA_ripper6_SOURCES += check_midi.cpp
EXTRA_ripper6_SOURCES += check_tbsa.cpp

WARNINGS = -Wall -Wextra -Wno-unused-parameter

AM_CPPFLAGS  = $(BOOST_CPPFLAGS)
AM_CPPFLAGS += -I $(top_srcdir)/include
AM_CPPFLAGS += $(libgamecommon_CPPFLAGS)
AM_CPPFLAGS += $(WX_CPPFLAGS)
AM_CPPFLAGS += $(WARNINGS)

AM_CXXFLAGS  = $(DEBUG_CXXFLAGS)
AM_CXXFLAGS += $(libgamecommon_CFLAGS)

AM_LDFLAGS  = $(BOOST_SYSTEM_LIBS)
AM_LDFLAGS += $(libgamecommon_LIBS)
