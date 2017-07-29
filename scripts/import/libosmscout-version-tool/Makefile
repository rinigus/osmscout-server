

######################################################
# Compiler and libraries
CXX := g++

######################################################

APPNAME := liboscmscout-version

SRCSUBDIR := src
OBJSUBDIR := obj

INCLUDE   += 
LIBRARIES += -losmscout

OBJS	= $(patsubst $(SRCSUBDIR)/%.cpp,$(OBJSUBDIR)/%.o,$(wildcard $(SRCSUBDIR)/*.cpp)) 

CXX_EXTRA_OPTIONS += -std=c++11
CXXFLAGS := -O2 -g $(EXTRA_OPTIONS) $(CXX_EXTRA_OPTIONS) $(INCLUDE)  

AR       = ar 
LD	 = g++ 

all: $(OBJSUBDIR) $(APPNAME)

clean:
	rm -rf core* $(APPNAME) $(OBJSUBDIR)

$(APPNAME): $(OBJS)
	@echo
	@echo "--------- LINKING --- $@ "
	rm -f $(APPNAME)
	$(LD) -o $@ $^ $(LIBRARIES) $(LD_EXTRA_OPTIONS)
	@echo
	@echo '--------- Make done '
	@echo

$(OBJSUBDIR):
	@echo
	@echo "--------- Making dir: $@ "
	mkdir -p $(OBJSUBDIR)
	@echo

$(OBJSUBDIR)/%.o: $(SRCSUBDIR)/%.cpp 
	@echo
	@echo "------------ $< "
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -o $@ $<
	@echo
