# Compiled Files
NAME    =   ft_ping

# Compiler and flags
CCXX        =   cc
CXXFLAGS    =   -Wall -Werror -Wextra -std=c2x -g -I includes

# Arguments
ARGS    =   localhost

# Directories
SRC_DIR = src
BIN_DIR = bin

# Source and Template Files
C_SRCS := $(SRC_DIR)/main.c \
		  $(SRC_DIR)/arg_parser.c \
		  $(SRC_DIR)/logger.c \
		  $(SRC_DIR)/ft_ping.c \

# Script Files
SCRIPT := $(SRC_DIR)/test_script.sh

# Object Files
OBJS    =   $(C_SRCS:$(SRC_DIR)/%.c=$(BIN_DIR)/%.o)

# Colors
DEFAULT = \033[0;39m
GRAY    = \033[0;90m
RED     = \033[0;91m
GREEN   = \033[0;92m
YELLOW  = \033[0;93m
BLUE    = \033[0;94m
MAGENTA = \033[0;95m
CYAN    = \033[0;96m
WHITE   = \033[0;97m
CURSIVE = \e[33;3m

all:        $(NAME)

$(NAME): $(OBJS)
			@echo "$(CURSIVE)Compiling...$(DEFAULT)"
			$(CCXX) $(CXXFLAGS) $(OBJS) -g -o $(NAME)
			@echo "$(GREEN)$(NAME) created successfully!$(DEFAULT)"

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
			@mkdir -p $(dir $@)
			$(CCXX) $(CXXFLAGS) -c $< -o $@

clean:
			@echo "$(BLUE)Cleaning...$(DEFAULT)"
			@rm -rf $(BIN_DIR)
			@echo "$(CYAN)Object Files Cleaned!$(DEFAULT)"

fclean:     clean
			@rm -f $(NAME)
			@echo "$(BLUE)Executables Cleaned!$(DEFAULT)"

re:         fclean all
			@echo "$(MAGENTA)Cleaned and rebuilt!$(DEFAULT)"

run: all
	./$(NAME) $(ARGS)

rerun: re run

test: all
	./$(SCRIPT)

gdb:    all
		gdb --args ./$(NAME) $(ARGS)

lldb:   all
		lldb ./$(NAME) $(ARGS)

valgrind:   all
			valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(NAME) $(ARGS)

.PHONY: re all clean fclean debug test