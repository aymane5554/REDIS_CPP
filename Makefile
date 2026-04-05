C++FLAGS=-Wall -Wextra -Werror
SRCS= Cache.cpp Server.cpp main.cpp Response.cpp Request.cpp Commands.cpp
INCS= Cache.hpp Server.hpp
OBJS= $(SRCS:%.cpp=%.o)
NAME= redihh

all: $(NAME)

$(NAME): $(OBJS)
	c++ $(C++FLAGS) $(OBJS) -o $@

%.o: %.cpp $(INCS)
	c++ $(C++FLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

