CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

SRC = Order.cpp \
      MarketOrder.cpp \
      Venue.cpp \
      ExecutionResult.cpp \
      SmartOrderRouter.cpp \
      Portfolio.cpp \
      PerformanceAnalyzer.cpp \
      main.cpp

OBJ    = $(SRC:.cpp=.o)
TARGET = sor_simulator

# ── Default target ─────────────────────────────────────────────────────────────
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# ── Convenience ────────────────────────────────────────────────────────────────
run: all
	./$(TARGET)

clean:
	rm -f $(OBJ) $(TARGET) results.txt