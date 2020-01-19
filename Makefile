
DEPFLAGS = -MT $@ -MMD -MP -MF .$@.d
CPPFLAGS = -g -Wall -std=c++17 -I./ $(DEPFLAGS)

OBJECTS=$(patsubst %.cpp,%.o,$(wildcard *.cpp))

test: $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS)

include $(wildcard .*.d)
