NAME= webserv
SRCS= main.cpp Server.cpp WebServer.cpp AConfig.cpp Config.cpp Location.cpp \
	config_parsing/check_bloc.cpp config_parsing/errors.cpp config_parsing/instructions.cpp \
	config_parsing/utils.cpp Client.cpp Request.cpp
SRC_DIR = srcs/
HEADER_DIR = includes/
HEADER_FILES = Server.hpp WebServer.hpp Client.hpp Request.hpp parsing.hpp AConfig.hpp \
	Config.hpp Location.hpp
HEADERS = $(addprefix $(HEADER_DIR), $(HEADER_FILES))
OBJS= $(addprefix $(SRC_DIR), $(SRCS:.cpp=.o))
CC= clang++
CFLAGS= -Wall -Wextra -Werror -std=c++98 -g3 -fsanitize=address
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
