CC?=gcc
LDLIBS?=-lopenal
ifeq ($(LIBAUDIO),1)
CFLAGS+=-DLIBAUDIO -g3 -O0
LDLIBS+=-laudio
else
CFLAGS+= -g3 -O0
LDLIBS+=-lalut
endif
APP:=openal-example
OBJS:= $(APP).o

all: $(APP)

%.o: %c
	$(CC) $(CFLAGS) -c $< -o $@

$(APP): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDLIBS)

clean:
	-rm -f $(OBJS) $(APP)

