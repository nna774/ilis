TARGET := ilis
all: $(TARGET)

CXXFLAGS := -Wall -Wextra --std=c++17
SRCS := main.cpp sexp.cpp parse.cpp eval.cpp env.cpp prelude.cpp

OBJS := $(SRCS:%.cpp=%.o)
DEPS := $(SRCS:%.cpp=%.d)
RM := rm -f
-include $(DEPS)

ilis: $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -MMD -MP $<

debug: CXXFLAGS += -DDEBUG -g
debug: clean
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)
