CXX = g++
CXXFLAGS= -c -v -Wall -mwindows
CXXFLAGS2= -c -v -Wall 

targetdir=bin

#TailOpts =-pg # This generates output for a profiler such as gprof
TailOpts= -O2 #optimize the code	
LinkerWinAPILibObj= -lsetupapi

CVdir=C:/Progra~2/OpenCV
nidir=C:/Progra~2/National\ Instruments/Vision
#OpenCV Include directories (for header files)
openCVincludes = -I$(CVdir)/cxcore/include -I$(CVdir)/otherlibs/highgui -I$(CVdir)/cv/include

nivisionincludes = -I$(nidir)/Include 

#3rd party statically linked objects
CVlibs=$(CVdir)/lib/cv.lib $(CVdir)/lib/highgui.lib $(CVdir)/lib/cxcore.lib
nivisionlibs=$(nidir)/Lib/MSVC/nivision.lib

all: $(targetdir)/find_fish_position.dll

$(targetdir)/find_fish_position.dll: find_fish_position_dll.o
	$(CXX) -shared -o $(targetdir)/find_fish_position.dll find_fish_position_dll.o  $(CVlibs) $(nivisionlibs) -Wl,--out-implib,find_fish_position_dll.a $(LinkerWinAPILibObj)

find_fish_position_dll.o: find_fish_position_dll.c find_fish_position_dll.h
	$(CXX) $(CXXFLAGS) -DBUILDING_FIND_FISH_POSITION_DLL find_fish_position_dll.c $(openCVincludes) $(nivisionincludes)

clean:

	rm -rf *.o 

