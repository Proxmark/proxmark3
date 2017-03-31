#-----------------------------------------------------------------------------
# This code is licensed to you under the terms of the GNU GPL, version 2 or,
# at your option, any later version. See the LICENSE.txt file for the text of
# the license.
#-----------------------------------------------------------------------------

CC = gcc
CXX = g++
LD = g++
TAR = tar
TARFLAGS = -C .. --ignore-failed-read -rvf
RM = rm -f
MV = mv

#COMMON_FLAGS = -m32
VPATH = ../common ../zlib
OBJDIR = obj

LDLIBS = -L/opt/local/lib -L/usr/local/lib -lreadline -lpthread -lm
LUALIB = ../liblua/liblua.a
LDFLAGS = $(COMMON_FLAGS)
CFLAGS = -std=c99 -D_ISOC99_SOURCE -I. -I../include -I../common -I../zlib -I/opt/local/include -I../liblua -Wall $(COMMON_FLAGS) -g -O4
LUAPLATFORM = generic

platform = $(shell uname)
ifneq (,$(findstring MINGW,$(platform)))
    CXXFLAGS = -I$(QTDIR)/include -I$(QTDIR)/include/QtCore -I$(QTDIR)/include/QtGui
    ifneq ($(wildcard $(QTDIR)/include/QtWidgets),)
        CXXFLAGS += -I$(QTDIR)/include/QtWidgets
        QTLDLIBS = -L$(QTDIR)/lib -lQt5Core -lQt5Gui -lQt5Widgets
    else
        QTLDLIBS = -L$(QTDIR)/lib -lQtCore4 -lQtGui4
    endif
    MOC = $(QTDIR)/bin/moc
    LUAPLATFORM = mingw
else
	CXXFLAGS = $(shell pkg-config --cflags Qt5Core Qt5Widgets 2>/dev/null) -Wall -O4
	QTLDLIBS = $(shell pkg-config --libs Qt5Core Qt5Widgets 2>/dev/null)
	MOC = $(shell pkg-config --variable=host_bins Qt5Core)/moc
	ifeq ($(QTLDLIBS), )
		CXXFLAGS = $(shell pkg-config --cflags QtCore QtGui 2>/dev/null) -Wall -O4
		QTLDLIBS = $(shell pkg-config --libs QtCore QtGui 2>/dev/null)
		MOC = $(shell pkg-config --variable=moc_location QtCore)
	endif
	ifeq ($(platform),Darwin)
		LUAPLATFORM = macosx
	else
		LUALIB +=  -ldl
		LDLIBS +=  -ltermcap -lncurses
		LUAPLATFORM = linux
	endif
endif

ifneq ($(QTLDLIBS),)
    QTGUIOBJS = $(OBJDIR)/proxgui.o $(OBJDIR)/proxguiqt.o $(OBJDIR)/proxguiqt.moc.o
    CFLAGS += -DHAVE_GUI
else
    QTGUIOBJS = $(OBJDIR)/guidummy.o
endif

# Flags to generate temporary dependency files
DEPFLAGS = -MT $@ -MMD -MP -MF $(OBJDIR)/$*.Td
# make temporary to final dependeny files after successful compilation
POSTCOMPILE = $(MV) -f $(OBJDIR)/$*.Td $(OBJDIR)/$*.d

CORESRCS = 	uart.c \
			util.c

CMDSRCS = 	crapto1/crapto1.c\
			crapto1/crypto1.c\
			mfkey.c\
			loclass/cipher.c \
			loclass/cipherutils.c \
			loclass/des.c \
			loclass/ikeys.c \
			loclass/elite_crack.c\
			loclass/fileutils.c\
			whereami.c\
			mifarehost.c\
			parity.c\
			crc.c \
			crc16.c \
			crc64.c \
			iso14443crc.c \
			iso15693tools.c \
			data.c \
			graph.c \
			ui.c \
			cmddata.c \
			lfdemod.c \
			cmdhf.c \
			cmdhf14a.c \
			cmdhf14b.c \
			cmdhf15.c \
			cmdhfepa.c \
			cmdhflegic.c \
			cmdhficlass.c \
			cmdhfmf.c \
			cmdhfmfu.c \
			cmdhftopaz.c \
			cmdhw.c \
			cmdlf.c \
			cmdlfawid.c \
			cmdlfcotag.c\
			cmdlfem4x.c \
			cmdlffdx.c \
			cmdlfgproxii.c \
			cmdlfhid.c \
			cmdlfhitag.c \
			cmdlfio.c \
			cmdlfindala.c \
			cmdlfjablotron.c \
			cmdlfnexwatch.c \
			cmdlfnoralsy.c \
			cmdlfparadox.c \
			cmdlfpcf7931.c \
			cmdlfpresco.c \
			cmdlfpyramid.c \
			cmdlfsecurakey.c \
			cmdlft55xx.c \
			cmdlfti.c \
			cmdlfviking.c\
			cmdlfvisa2000.c\
			cmdparser.c \
			cmdmain.c \
			scripting.c\
			cmdscript.c\
			pm3_binlib.c\
			pm3_bitlib.c\
			aes.c\
			protocols.c\
			sha1.c\
			cmdcrc.c\
			reveng/reveng.c\
			reveng/cli.c\
			reveng/bmpbit.c\
			reveng/model.c\
			reveng/poly.c\
			reveng/getopt.c\

ZLIBSRCS = deflate.c adler32.c trees.c zutil.c inflate.c inffast.c inftrees.c
ZLIBFLAGS = -DZ_SOLO -DZ_PREFIX -DNO_GZIP -DZLIB_PM3_TUNED 
#-DDEBUG -Dverbose=1

QTGUISRCS = proxgui.cpp proxguiqt.cpp proxguiqt.moc.cpp guidummy.cpp

COREOBJS = $(CORESRCS:%.c=$(OBJDIR)/%.o)
CMDOBJS = $(CMDSRCS:%.c=$(OBJDIR)/%.o)
ZLIBOBJS = $(ZLIBSRCS:%.c=$(OBJDIR)/%.o)

BINS = proxmark3 flasher fpga_compress
WINBINS = $(patsubst %, %.exe, $(BINS))
CLEAN = $(BINS) $(WINBINS) $(COREOBJS) $(CMDOBJS) $(ZLIBOBJS) $(QTGUIOBJS) $(OBJDIR)/*.o *.moc.cpp

all: lua_build $(BINS)

all-static: LDLIBS:=-static $(LDLIBS)
all-static: proxmark3 flasher fpga_compress

proxmark3: LDLIBS+=$(LUALIB) $(QTLDLIBS)
proxmark3: $(OBJDIR)/proxmark3.o $(COREOBJS) $(CMDOBJS) $(QTGUIOBJS) lualibs/usb_cmd.lua
	$(LD) $(LDFLAGS) $(OBJDIR)/proxmark3.o $(COREOBJS) $(CMDOBJS) $(QTGUIOBJS) $(LDLIBS) -o $@

flasher: $(OBJDIR)/flash.o $(OBJDIR)/flasher.o $(COREOBJS)
	$(LD) $(LDFLAGS) $^ $(LDLIBS) -o $@

fpga_compress: $(OBJDIR)/fpga_compress.o $(ZLIBOBJS)
	$(LD) $(LDFLAGS) $(ZLIBFLAGS) $^ $(LDLIBS) -o $@

proxguiqt.moc.cpp: proxguiqt.h
	$(MOC) -o$@ $^

lualibs/usb_cmd.lua: ../include/usb_cmd.h
	awk -f usb_cmd_h2lua.awk $^ > $@
	
clean:
	$(RM) $(CLEAN)
	cd ../liblua && make clean

tarbin: $(BINS)
	$(TAR) $(TARFLAGS) ../proxmark3-$(platform)-bin.tar $(BINS:%=client/%) $(WINBINS:%=client/%)

lua_build:
	@echo Compiling liblua, using platform $(LUAPLATFORM)
	cd ../liblua && make $(LUAPLATFORM)

.PHONY: all clean

%.o: %.c
$(OBJDIR)/%.o : %.c $(OBJDIR)/%.d
	$(CC) $(DEPFLAGS) $(CFLAGS) $(ZLIBFLAGS) -c -o $@ $<
	$(POSTCOMPILE)

%.o: %.cpp
$(OBJDIR)/%.o : %.cpp $(OBJDIR)/%.d
	$(CXX) $(DEPFLAGS) $(CXXFLAGS) -c -o $@ $<
	$(POSTCOMPILE)


#$(CMDOBJS) $(COREOBJS): $(notdir $(%.c)) %.d
#	$(CC) $(DEPFLAGS) $(CFLAGS) -c -o $@ $<
#	$(POSTCOMPILE)

#$(ZLIBOBJS): $(notdir $(%.c)) %.d
#	$(CC) $(DEPFLAGS) $(CFLAGS) $(ZLIBFLAGS) -c -o $@ $<
#	$(POSTCOMPILE)

#$(QTGUIOBJS): $(notdir $(%.cpp)) %.d
#	$(CXX) $(DEPFLAGS) $(CXXFLAGS) -c -o $@ $<
#	$(POSTCOMPILE)

DEPENDENCY_FILES = $(patsubst %.c, $(OBJDIR)/%.d, $(CORESRCS) $(CMDSRCS) $(ZLIBSRCS)) \
	$(patsubst %.cpp, $(OBJDIR)/%.d, $(QTGUISRCS)) \
	$(OBJDIR)/proxmark3.d $(OBJDIR)/flash.d $(OBJDIR)/flasher.d $(OBJDIR)/fpga_compress.d

$(DEPENDENCY_FILES): ;
.PRECIOUS: $(DEPENDENCY_FILES)

-include $(DEPENDENCY_FILES)

