CXX = clang++
CXXFLAGS = -Wall -Wextra -O0 -g3 -ggdb3 -std=c++23 -MMD -MP

SRCDIR = .
BUILDDIR = build
SRC = $(wildcard $(SRCDIR)/*.cpp)
OBJ = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRC))
DEP = $(OBJ:.o=.d)
EXEC = $(BUILDDIR)/reic

all: $(EXEC)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(EXEC): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

-include $(DEP)

clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean
