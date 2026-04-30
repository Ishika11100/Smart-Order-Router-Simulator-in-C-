CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I/mingw64/include
LDFLAGS  = -L/mingw64/lib -lcurl

TARGET = sor_simulator

SRCS = Main.cpp \
       Order.cpp \
       MarketOrder.cpp \
       Venue.cpp \
       ExecutionResult.cpp \
       SmartOrderRouter.cpp \
       Portfolio.cpp \
       PerformanceAnalyzer.cpp \
       MarketDataFetcher.cpp \
       OrderSplitter.cpp

OBJS = $(SRCS:.cpp=.o)

HEADERS = Order.h MarketOrder.h Venue.h ExecutionResult.h \
          SmartOrderRouter.h Portfolio.h PerformanceAnalyzer.h \
          MarketData.h RegulatoryFees.h MarketDataFetcher.h OrderSplitter.h

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).exe

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run