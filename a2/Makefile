SOURCE=parser.c
SOURCE2=search.c
NAME=$(basename $(SOURCE))
NAME2=$(basename $(SOURCE2))

all: $(NAME) $(NAME2)

$(NAME): $(SOURCE)
	cc -std=gnu11 -Wall -Werror $(SOURCE) -o $(NAME) -g -lm

$(NAME2): $(SOURCE2)
	cc -std=gnu11 -Wall -Werror $(SOURCE2) -o $(NAME2) -g -lm

.PHONY: clean test
clean:
	rm -rf $(NAME) $(NAME).dSYM
	rm -rf $(NAME2) $(NAME2).dSYM

