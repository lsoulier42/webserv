NAME= webserv
SRCS= main.cpp Server.cpp WebServer.cpp
SRC_DIR = srcs/
HEADER_DIR = includes/
HEADER_FILES = Server.hpp Request.hpp
HEADERS = $(addprefix $(HEADER_DIR), $(HEADER_FILES))
OBJS= $(addprefix $(SRC_DIR), $(SRCS:.cpp=.o))
CC= clang++
CFLAGS= -Wall -Wextra -Werror -std=c++98
INCLUDE= -I $(HEADER_DIR)

./$(SRC_DIR)%.o: ./$(SRC_DIR)%.cpp $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@
$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
all: $(NAME)
clean:
	rm -f $(OBJS)
fclean: clean
	rm -f $(NAME)
re: fclean all
.PHONY: all clean fclean re
