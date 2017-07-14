TARGET=kcommand.exe
ASM=WASM
CC	= WCC386
RC	= WRC
HC	= HC31
DISASM	= WDIS
LNK	= WLINK

ODIR=OBJ\

INCLUDE=$(WC)\H;$(WC)\H\NT

COPTS = -zk0 -bt=nt -mf

DEFINES	=
RESFLAGS=  -r
OBJS	= &
	$(ODIR)base.obj &
	$(ODIR)wnd.obj &
	$(ODIR)opt.obj

DLL_OBJS = &
	$(ODIR)dll.obj

LINK_OBJS = &
	$(ODIR)base, &
	$(ODIR)wnd, &
	$(ODIR)opt

DLL_LINK_OBJS = &
	$(ODIR)dll

RES=$(ODIR)KC.RES

LINK_FLAGS= NAME $(TARGET) &
	LIBP $(WC)\lib386;$(WC)\lib386\nt &
	LANG JA &
	FORM WINDOWS NT RU WIN=4.0 &
	SORT COMMIT HEAP=1M COMMIT STACK=4K &
	OP STACK=4M,HEAPSIZE=4M,Map=MAPFILE.TXT,EL,&
	RES=$(ODIR)KC.RES,ALIGNMENT=512,OBJA=4096, &
	DE NOTEXIT,VERS=1.0, &
	MODN=kc,VERBOSE,STATICS

DLL_LINK_FLAGS= NAME KCHOOK.DLL &
	LIBP $(WC)\lib386;$(WC)\lib386\nt &
	LANG JA &
	SYS NT_DLL &
#	SEGMENT CLASS DATA SHARED &
	SEGMENT TYPE DATA SHARED &
	EXPORT InstallHook='_InstallHook@8', UninstallHook='_UninstallHook@0', LaserPause='_pause@4'&
	SORT COMMIT HEAP=1M COMMIT STACK=4K &
	OP STACK=4M,HEAPSIZE=4M,Map=MAPDLL.TXT,EL, &
	ALIGNMENT=512,OBJA=4096, &
	DE kchook,VERS=1.0, &
	MODN=NOTEXIT,VERBOSE,STATICS


LINK_LIBS = LIB kernel32,user32,gdi32,shell32
DLL_LINK_LIBS = LIB kernel32,user32

OPTION =

!ifdef D
CFLAGS = -D_DEBUG -3r -d2 -db -hd -s -w=4
LINK_OPTS= DEBUG DWARF
DLL_LINK_OPTS= DEBUG DWARF
!else
CFLAGS = -DRELEASE -5r -oabhkrs -s -w=4 -d0
LINK_OPTS = OP NOD,NOR,START='Entry_'
DLL_LINK_OPTS = OP NOD,START='_DllEntryPoint@12'
!endif

.EXTENSIONS:
.EXTENSIONS: .exe .lib .obj .asm .c .cpp .res .rc .h

.c.obj:
	$(CC) -i=$(INCLUDE) $(COPTS) $(CFLAGS) $(OPTION) -fo$@ $[@

.asm.obj:
	$(ASM) $(AFLAGS) -fo$@ $[@

.rc.res:
	$(RC) $(RCDEFINES) $(RESFLAGS) -i=$(INCLUDE) -fo$@ $[@

all :: $(TARGET) kchook.dll


$(TARGET): $(OBJS) $(RES)
	SET WATCOM=$(WC)
	-del $(TARGET)
	$(LNK) @<<
	$(LINK_FLAGS)
	$(LINK_OPTS)
	FILE $(LINK_OBJS)
	$(LINK_LIBS)
<<
	SET WATCOM=

kchook.dll: $(DLL_OBJS)
	SET WATCOM=$(WC)
	-del kchook.dll
	$(LNK) @<<
	$(DLL_LINK_FLAGS)
	$(DLL_LINK_OPTS)
	FILE $(DLL_LINK_OBJS)
	$(DLL_LINK_LIBS)
<<
	SET WATCOM=

$(ODIR)DLL.OBJ : DLL.C
	$(CC) -i=$(INCLUDE) $(COPTS) $(CFLAGS) $(OPTION) -fo$@ $[@

$(ODIR)NOTEXIT.OBJ : NOTEXIT.C
	$(CC) -i=$(INCLUDE) $(COPTS) $(CFLAGS) $(OPTION) -fo$@ $[@

$(ODIR)BW.RES : BW.RC
	

