#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "card.h"

void print_card(CARD_T *);
void search_for_card(char *);
void free_indices();

INDEX_T **indices = NULL;
size_t total_indices = 0;

int search_comparator(const void *a, const void *b) {
	char *key = (char *) a;
	INDEX_T *b_element = *(INDEX_T **) b;
	return strcmp(key, b_element->card_name);
}

FILE *cards_in;

int main (int argc, char **argv) {

	FILE *index_in = fopen("index.bin", "rb");
    cards_in = fopen("cards.bin", "rb");

    if (!index_in || !cards_in) return -1;

    // first thing to do is read all the contents of the files into an array so we can search
    size_t total_cards;
    // first read the total number of cards into total cards
    fread(&total_cards, sizeof(size_t), 1, index_in);

    // then loop through all the cards to get the card_name and offset in index.bin
    for (int i = 0; i < total_cards; i++) {
        int card_name_size;
        off_t offset;
        
        // read in the card name size
        fread(&card_name_size, sizeof(int), 1, index_in);

        // allocate address for a new card name with the card name size + 1, +1 for the null terminator
        char *card_name_in = malloc(card_name_size + 1);
        card_name_in[card_name_size] = '\0';

        // read the card name into the new address, then read the offset into offset
        fread(card_name_in, card_name_size, 1, index_in);
        fread(&offset, sizeof(off_t), 1, index_in);

        // reallocate the indices array, the element in the array, then store the card_name, offset, and increment the total indices
        indices = realloc(indices, sizeof(INDEX_T *) * (total_indices + 1));
        indices[i] = malloc(sizeof(INDEX_T));
        indices[i]->card_name = card_name_in;
        indices[i]->offset = offset;
        total_indices++;
    }
    
    // if isatty(0) then we are expecting user input
    if (isatty(0)) {
        printf(">> ");

        char *usr_input = (char *)malloc(sizeof(char *));
        size_t input_size = 1;

        getline(&usr_input, &input_size, stdin);
        // get rid of new line in user input
        usr_input[strlen(usr_input) - 1] = '\0';

        // do until the user enters 'q' 
        while (strcmp(usr_input, "q") != 0) {
            
            search_for_card(usr_input);

            // reget user input
            printf(">> ");
            getline(&usr_input, &input_size, stdin);
            // get rid of new line in user input
            usr_input[strlen(usr_input) - 1] = '\0';
        }
        free(usr_input);
    } else {
        FILE *infile = stdin;
        char *infileptr = NULL;
		size_t infile_size = 0;
		while (getline(&infileptr, &infile_size, infile) != -1) {
            
            // Get rid of '\r\n' if input contains so and get rid of '\n' if input only contains so.
            // Even though this seems a little inefficient to copy two of the same lines it'll cover more unique situations
            if (strlen(infileptr) > 1) {
                if (strlen(infileptr) > 2) {
                    if (infileptr[strlen(infileptr) - 2] == '\r') infileptr[strlen(infileptr) - 2] = '\0';
                    else if (infileptr[strlen(infileptr) - 1] == '\n') infileptr[strlen(infileptr) - 1] = '\0';
                }
                else if (infileptr[strlen(infileptr) - 1] == '\n') infileptr[strlen(infileptr) - 1] = '\0';
            }

            // print line input
            printf(">> %s\n", infileptr);

            // if q is entered, break out of the loop
            if (strcmp(infileptr, "q") == 0) break;
            search_for_card(infileptr);
            
		}
        // free the ptr
		free(infileptr);

    }

    // clean up index array and close files
    free_indices();
    fclose(cards_in);
    fclose(index_in);
    return 0;

}

void search_for_card(char *input) {

    // we will binary search the indices array for the card name to see if it's a valid card
    INDEX_T **result = bsearch(input, indices, total_indices, sizeof(INDEX_T *), search_comparator);
    
    // if not a valid card, spit out prompt to user
    if (!result) {
        printf("./search: '%s' not found!\n", input);
        return;
    }

    // Now fetch the card information from cards.bin

    // First move the pointer to the where the information is stored in cards.bin
    fseeko(cards_in, (*result)->offset, SEEK_SET);

    // allocate memory for a card
    CARD_T *card = malloc(sizeof(CARD_T));

    // create a variable to hold the text size and set the card->name field to the result's card name
    int text_size;
    card->name = (*result)->card_name;

    // read in the id, cost, type, class, rarity, and text size.
    fread(&card->id, 4, 1, cards_in);
	fread(&card->cost, 4, 1, cards_in);
	fread(&card->type, 4, 1, cards_in);
	fread(&card->class, 4, 1, cards_in);
	fread(&card->rarity, 4, 1, cards_in);
	fread(&text_size, 4, 1, cards_in);

    // allocate memory for a card_text field and put a null terminator at the text size position
    char *card_text = malloc(text_size + 1);
    card_text[text_size] = '\0';

    // read the card text into the card_text field and set the card->text field to card_text
	fread(card_text, text_size, 1, cards_in);
    card->text = card_text;

    // get the attack and health
	fread(&card->attack, 4, 1, cards_in);
	fread(&card->health, 4, 1, cards_in);

    // print the card, then free the card text field and the card itself
    print_card(card);
    free(card->text);
    free(card);
}

void free_indices() {
    for (int i = 0; i < total_indices; i++) {
        free(indices[i]->card_name);
        free(indices[i]);
    } 
    free(indices);
}

void print_card(CARD_T *card) {
	printf("%-29s %2d\n", card->name, card->cost);
	unsigned length = 15 - strlen(class_str[card->class]);
	unsigned remainder = length % 2;
	unsigned margins = length / 2;
	unsigned left = 0;
	unsigned right = 0;
	if (remainder) {
		left = margins + 2;
		right = margins - 1;
	} else {
		left = margins + 1;
		right = margins - 1;
	}
	printf("%-6s %*s%s%*s %9s\n", type_str[card->type], left, "", class_str[card->class], right, "", rarity_str[card->rarity]);
	printf("--------------------------------\n");
	printf("%s\n", card->text);
	printf("--------------------------------\n");
	printf("%-16d%16d\n\n", card->attack, card->health);
}