NAME    = ft_ping


SRCS    = ft_ping_main.c ft_ping_init.c ft_ping_utils.c ft_ping_parsing.c ft_ping_receive.c
INCLUDES = ft_ping.h
OBJS    =   ${SRCS:.c=.o}

CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -lm
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes


GREEN   = \033[32m
YELLOW  = \033[33m
WHITE   = \033[37m
RED     = \033[0;31m


all:    ${NAME}


${NAME}:    ${OBJS}
			@echo "${GREEN}Objects OK"
			@tput cuu1 && tput dl1
			@echo "${WHITE}Compiling ft_ping..."
			@${CC} ${INCLUDES}  ${CFLAGS} ${OBJS} -o ${NAME}
			@tput cuu1 && tput dl1
			@echo "${GREEN}ft_ping OK${WHITE}"


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
		@echo "${WHITE}Cleaning ft_ping"
		@rm -f ${NAME}
		@tput cuu1 && tput dl1
		@echo "${YELLOW}ft_ping cleaned${WHITE}"


re: fclean all


.PHONY: all clean fclean re


