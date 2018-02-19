SRCS_PATHS = .
SRCS = $(foreach p,$(SRCS_PATHS),$(wildcard $(p)/*.cpp))
OBJS_PATH = objs
OBJS = $(foreach f,$(SRCS),$(OBJS_PATH)/$(basename $(notdir $(f))).o)
CFLAGS = -c -std=c++11 -I..

TARGET = test

$(TARGET) : $(OBJS)
	$(CC) -o $@ $(OBJS) -lstdc++

$(OBJS_PATH)/%.o : ./%.cpp ../callbacks.h
	mkdir -p $(OBJS_PATH) && $(CC) $(CFLAGS) -o $@ $<

run :
	cd ../example && ../osx/server

clean : 
	rm $(OBJS)

$(info OBJS = $(OBJS))
