# Build the RTOS project

# Overall options
CPPDIR = rtx
HDIR = include
BINDIR = bin
CC = g++

# RTX options
RTXDEPENDS = set.o 
RTXOPTS = -g
RTXFILES = $(CPPDIR)/*.cpp $(CPPDIR)/set.o 
RTXOUT = RTX.out

# Helper options
HELPEROPTS = -g -lncurses
HELPERDIR = helpers
KEYBOARDOUT = KB.out
KBHELPERFILES = KeyboardHelper.cpp
DISPLAYOUT = DSP.out
DPHELPERFILES = DisplayHelper.cpp

# Build all three components
all: $(RTXOUT) $(KEYBOARDOUT) $(DISPLAYOUT)
	@echo "RTOS built successfully at:"
	@date

$(RTXOUT): $(RTXDEPENDS)
	@echo "Building RTX..."
	$(CC) $(RTXOPTS) $(RTXFILES) -I$(HDIR) -o $(BINDIR)/$(RTXOUT)
	@echo

$(KEYBOARDOUT):
	@echo "Building Keyboard Helper process..."
	$(CC) $(HELPEROPTS) $(HELPERDIR)/$(KBHELPERFILES) -I$(HDIR) -o $(BINDIR)/$(KEYBOARDOUT)
	@echo

$(DISPLAYOUT):
	@echo "Building Display Helper process..."
	$(CC) $(HELPEROPTS) $(HELPERDIR)/$(DPHELPERFILES) -I$(HDIR) -o $(BINDIR)/$(DISPLAYOUT)
	@echo

set.o:
	$(CC) -c $(RTXOPTS) $(CPPDIR)/set.s -o $(CPPDIR)/set.o
