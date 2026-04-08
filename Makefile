C++FLAGS=-Wall -Wextra -Werror
SRCS= Cache.cpp Server.cpp main.cpp Response.cpp Request.cpp Commands.cpp
INCS= Cache.hpp Server.hpp
OBJS= $(SRCS:%.cpp=obj/%.o)
NAME= redihh

all: $(NAME)

$(NAME): obj $(OBJS)
	c++ $(C++FLAGS) $(OBJS) -o $@

obj:
	mkdir obj

obj/%.o: %.cpp $(INCS)
	c++ $(C++FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS) obj

fclean: clean
	rm -f $(NAME)

re: fclean all

