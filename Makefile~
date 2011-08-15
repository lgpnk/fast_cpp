# Note: the following 2 lines will use the Axis build system,
# which sets up all the $(CC) macros.
# To cross compile for a camera or video encoder, 
# you must define  AXIS_BUILDTYPE 
# This can be done on in 3 ways:
# - on the command line calling make
# - in this file before including the Rules.axis
# - in the hidden file .target-makefrag (if it exists)
#
# use create-package.sh to generate a .target-makefrag (if it does not exist)
## AXIS_BUILDTYPE ?=
AXIS_USABLE_LIBS = UCLIBC GLIBC
include $(AXIS_TOP_DIR)/tools/build/Rules.axis

PROGS     = fastcpp

CFLAGS   += -Wall -g -O2
LDFLAGS  += -lcapture -lparam -lnet_http 	    -Wl,-Bstatic,-Bdynamic
ifneq ($(AXIS_OPT_DEBUG),y)
ifneq ($(AXIS_OPT_STATIC),y)
# Strip the binaries when building unless debug or static
LDFLAGS  += -s
endif
endif

SRCS      = 	main.cpp CaptureHandler.cpp HttpHandler.cpp ParamHandler.cpp SignalHandler.cpp Fast.cpp Fast9.cpp Nonmax.cpp

OBJS      = $(SRCS:.cpp=.o)

all:	$(PROGS)

$(PROGS): $(OBJS)
	$(CXX) $(LDFLAGS) $^ $(LIBS) $(LDLIBS) -o $@

# Install the package on the camera
install:	$(PROGS)
	create-package.sh
	install-onto-target.sh

clean:
	rm -f $(PROGS) *.o core *.eap
