.PHONY : compile clean test

ifeq ($(OS),Windows_NT)
  EXE=.exe
  OBJ=.obj
  CC=cl /nologo /O2 /Fe:
  CCC=cl /nologo /EHsc /O2 /Fe:
  RM=del 2>nul
  TIME=timer
  X=
  NULL=nul
else
  EXE=
  OBJ=.o
  CC=cc -O2 -o
  CCC=c++ -O2 -o
  RM=rm -f
  TIME=time -p
  X=./
  NULL=/dev/null
endif

PROGS=fastcopy${EXE} slowcopy${EXE} fastcopypp${EXE} slowcopypp${EXE} gendat${EXE}

DATA=gen.dat.20m


compile : ${PROGS}

%${EXE} : %.c
	@${CC} $@ $<

%${EXE} : %.cpp
	@${CCC} $@ $<

clean :
	@${RM} ${PROGS} *${OBJ}

${DATA} :
	${X]gendat 20000000 >$@

test : compile ${DATA}
	crc ${DATA}
	${X}fastcopy   < ${DATA} | crc
	${X}fastcopypp < ${DATA} | crc

perf : compile ${DATA}
	${TIME} ${X}fastcopy   < ${DATA} >${NULL}
	${TIME} ${X}slowcopy   < ${DATA} >${NULL}
	${TIME} ${X}fastcopypp < ${DATA} >${NULL}
	${TIME} ${X}slowcopypp < ${DATA} >${NULL}
