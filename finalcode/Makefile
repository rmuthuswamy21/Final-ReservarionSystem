CXX = g++
CXXFLAGS = -Wall -std=c++11
SRCS = main.cpp Event.cpp Facility.cpp FacilityManager.cpp Reservation.cpp Ticket.cpp User.cpp Resident.cpp NonResident.cpp CityUser.cpp Organization.cpp
HEADERS = Event.h Facility.h FacilityManager.h Reservation.h Ticket.h User.h Resident.h NonResident.h CityUser.h Organization.h make_unique.h
OBJS = $(SRCS:.cpp=.o)
EXEC = program

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJS) $(EXEC)

.PHONY: all clean

