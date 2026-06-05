CXX ?= g++
PKG_CONFIG ?= pkg-config
ARCH ?= x86-64
BUILD_DIR ?= build
TARGET ?= RuneGenerator

CXXFLAGS ?= -std=c++20 -O2 -pipe -Wall -Wextra -Wpedantic -march=$(ARCH)
CPPFLAGS += $(shell $(PKG_CONFIG) --cflags x11)
LDLIBS += $(shell $(PKG_CONFIG) --libs x11)

SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
DEPS := $(OBJECTS:.o=.d)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LDLIBS)

$(BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

-include $(DEPS)
