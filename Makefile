NAME = webserv

SRCS_FILES = main.cpp Server.cpp WebServer.cpp AConfig.cpp Config.cpp Location.cpp \
	config_parsing/check_bloc.cpp config_parsing/errors.cpp config_parsing/instructions.cpp \
	config_parsing/utils.cpp Client.cpp Request.cpp
HEADER_FILES = Server.hpp WebServer.hpp Client.hpp Request.hpp parsing.hpp AConfig.hpp \
	Config.hpp Location.hpp

SRCS_DIR = ./srcs/
SRCS_SUB_DIR = config_parsing/
HEADER_DIR = ./includes/
OBJS_DIR = ./objs/
OBJS_SUB_DIR = $(addprefix $(OBJS_DIR), $(SRCS_SUB_DIR))

HEADERS = $(addprefix $(HEADER_DIR), $(HEADER_FILES))
SRCS = $(addprefix $(SRCS_DIR), $(SRCS_FILES))
OBJS = $(addprefix $(OBJS_DIR), $(SRCS_FILES:.cpp=.o))

CC = clang++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -g3 -fsanitize=address
INCLUDE = -I $(HEADER_DIR)

$(OBJS_DIR)%.o: $(SRCS_DIR)%.cpp $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(NAME): $(OBJS_SUB_DIR) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

all: $(NAME)

$(OBJS_SUB_DIR):
	mkdir -p ${OBJS_SUB_DIR}

clean:
	rm -r $(OBJS_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
