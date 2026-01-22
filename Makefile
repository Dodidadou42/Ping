NAME    = ft_ping

SRCS    = srcs/ft_ping_main.c srcs/ft_ping_init.c \
			srcs/ft_ping_utils.c srcs/ft_ping_receive.c
INCLUDES = includes/ft_ping.h
OBJS    =   ${SRCS:.c=.o}

CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -lm

GREEN   = \033[32m
YELLOW  = \033[33m
WHITE   = \033[37m
RED     = \033[0;31m

all:    ${NAME}

${NAME}:    ${OBJS}
			@echo "${GREEN}Objects OK"
			@tput cuu1 && tput dl1
			@echo "${WHITE}Compiling ${NAME}..."
			@${CC} ${INCLUDES}  ${CFLAGS} ${OBJS} -o ${NAME}
			@tput cuu1 && tput dl1
			@echo "${GREEN}${NAME} OK${WHITE}"

.c.o:  
		@echo "${WHITE}Compiling object $<"
		@${CC} ${CFLAGS} -c $< -o ${<:.c=.o}
		@tput cuu1 && tput dl1

clean:
		@echo "${WHITE}Cleaning objects..."
		@rm -f ${OBJS}
		@tput cuu1 && tput dl1
		@echo "${YELLOW}Objects cleaned"

fclean: clean
		@echo "${WHITE}Cleaning ${NAME}"
		@rm -f ${NAME}
		@tput cuu1 && tput dl1
		@echo "${YELLOW}${NAME} cleaned${WHITE}"

re: fclean all

.PHONY: all clean fclean re


