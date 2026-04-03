SRCS= Cache.cpp Server.cpp main.cpp
INCS= Cache.hpp Server.hpp
OBJS= $(SRCS:%.cpp=%.o)
NAME= redihh

all: $(NAME)

$(NAME): $(OBJS)
	c++ $(OBJS) -o $@

%.o: %.cpp $(INCS)
	c++ -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

