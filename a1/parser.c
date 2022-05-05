#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "card.h"

/*
 * I've left these definitions in from the
 * solution program. You don't have to
 * use them, but the `dupe_check()` function
 * unit test expects certain values to be
 * returned for certain situations!
 */
#define DUPE -1
#define NO_DUPE -2

/*
 * These are the special strings you need to
 * print in order for the text in the terminal
 * to be bold, italic or normal (end)
 */
#define BOLD "\e[1m"
#define ITALIC "\e[3m"
#define END "\e[0m"

/*
 * You will have to implement all of these functions
 * as they are specifically unit tested by Mimir
 */
int dupe_check(unsigned, char*);
char *fix_text(char*);
void free_card(CARD_T*);
CARD_T *parse_card(char*);
void print_card(CARD_T*);
int sort_comparator(const void *a, const void *b) {
	CARD_T *a_element = *(CARD_T **) a;
	CARD_T *b_element = *(CARD_T **) b;
	return strcmp(a_element->name, b_element->name);
}

int comparator(const void *a, const void *b) {
	char *key = (char *) a;
	CARD_T *b_element = *(CARD_T **) b;
	return strcmp(key, b_element->name);
}

/*
 * We'll make these global again, to make
 * things a bit easier
 */
CARD_T **cards = NULL;
size_t total_cards = 0;

int main(int argc, char **argv) {
	// TODO: 1. Open the files
	if (argc < 2) {
		printf("Error: no file given\n");
		return -1;
	}
	char *file_in = argv[1];
	FILE *fp = fopen(file_in, "r");
	if (!fp) {
		printf("Error opening file.\n");
		return -1;
	} 

	char *lineptr = NULL;
	size_t n = 0;
	// get the first line so we ignore the header when looping
	getline(&lineptr, &n, fp);

	//       2. Read lines from the file...         
	while (getline(&lineptr, &n, fp) != -1) {
		//  a. for each line, `parse_card()`
		CARD_T *card_res = parse_card(lineptr);
		if (!card_res) continue;
		// reallocate array
		total_cards++;
		cards = realloc(cards, sizeof(CARD_T *) * total_cards);
		//  b. add the card to the array
		cards[total_cards - 1] = card_res;
	}

	//       3. Sort the array
	qsort(cards, total_cards, sizeof(CARD_T *), sort_comparator);

	//       4. Print and free the cards
	for (int i = 0; i < total_cards; i++) {
		print_card(cards[i]);
		free_card(cards[i]);
	}


	//       5. Clean up!
	free(lineptr);
	free(cards);
	fclose(fp);
	return 0;
}

/*
 * This function has to return 1 of 3 possible values:
 *     1. NO_DUPE (-2) if the `name` is not found
 *        in the `cards` array
 *     2. DUPE (-1) if the `name` _is_ found in
 *        the `cards` array and the `id` is greater
 *        than the found card's id (keep the lowest one)
 *     3. The last case is when the incoming card has
 *        a lower id than the one that's already in
 *        the array. When that happens, return the
 *        index of the card so it may be removed...
 */
int dupe_check(unsigned id, char *name) {
	CARD_T **result = lfind(name, cards, &total_cards, sizeof(CARD_T *), comparator);
	if (result) {
		if (id > result[0]->id) return -1;
		// return index of card if id < id in array
		return result - cards;
	}
	return -2;
}

/*
 * This function has to do _five_ things:
 *     1. replace every "" with "
 *     2. replace every \n with `\n`
 *     3. replace every </b> and </i> with END
 *     4. replace every <b> with BOLD
 *     5. replace every <i> with ITALIC
 *
 * The first three are not too bad, but 4 and 5
 * are difficult because you are replacing 3 chars
 * with 4! You _must_ `realloc()` the field to
 * be able to insert an additional character else
 * there is the potential for a memory error!
 */
char *fix_text(char *text) {
	char *ptr;
	// move everything over by one to eliminate one of the quotes
	for (ptr = strstr(text, "\"\""); ptr; ptr = strstr(ptr, "\"\"")) memmove(&ptr[1], &ptr[2], strlen(&ptr[2]) + 1);
	
	for (ptr = strstr(text, "\\n"); ptr; ptr = strstr(ptr, "\\n")) {
		// move everything over by one in the pointer, then copy a newline at the beginning of the pointer
		memmove(&ptr[1], &ptr[2], strlen(&ptr[2]) + 1);
		memcpy(&ptr[0], "\n", 1);
	}
	
	// same amount of characters, no memory moving necessary
	for (ptr = strstr(text, "</"); ptr; ptr = strstr(ptr, "</")) memcpy(&ptr[0], END, 4);

	// reallocate text field size, reget the pointer, memmove everything to the right by 1, insert BOLD escape code
	for (ptr = strstr(text, "<b>"); ptr; ptr = strstr(ptr, "<b>")) {
		text = realloc(text, strlen(text) + 2);
		ptr = strstr(text, "<b>");
		memmove(&ptr[2], &ptr[1], strlen(&ptr[1]) + 1);
		memcpy(&ptr[0], BOLD, 4);
	}

	// reallocate text field size, reget the pointer, memmove everything to the right by 1, insert ITALIC escape code
	for (ptr = strstr(text, "<i>"); ptr; ptr = strstr(ptr, "<i>")) {
		text = realloc(text, strlen(text) + 2);
		ptr = strstr(text, "<i>");
		memmove(&ptr[2], &ptr[1], strlen(&ptr[1]) + 1);
		memcpy(&ptr[0], ITALIC, 4);
	}

	return text;
}

/*
 * This short function simply frees both fields
 * and then the card itself
 */
void free_card(CARD_T *card) {
	free(card->name);
	free(card->text);
	free(card);
}

/*
 * This is the tough one. There will be a lot of
 * logic in this function. Once you have the incoming
 * card's id and name, you should call `dupe_check()`
 * because if the card is a duplicate you have to
 * either abort parsing this one or remove the one
 * from the array so that this one can go at the end.
 *
 * To successfully parse the card text field, you
 * can either go through it (and remember not to stop
 * when you see two double-quotes "") or you can
 * parse backwards from the end of the line to locate
 * the _fifth_ comma.
 *
 * For the fields that are enum values, you have to
 * parse the field and then figure out which one of the
 * values it needs to be. Enums are just numbers!
 */
CARD_T *parse_card(char *line) {

	// get id
	char *stringp = line;
	char *token = strsep(&stringp, ",");
	int id = atoi(token);

	// skip first double quote
	stringp++;
	// get name
	token = strsep(&stringp, "\"");

	// Duplicate card checking
	if (total_cards > 0) {
		int dupe_return = dupe_check(id, token);
		// if dupe return was -2, no dupe was found so we can continue adding the card
		if (dupe_return >= -1) {
			// return null and do not add card if a dupe was found but the id was greater
			if (dupe_return == -1) return NULL;
			/* Now we have a card that has a lower id than one found in the array, so we will free the card at the index returned,
				then shift everything over in the array to the left by 1. Then decrement the total cards so when we increment 
				the total cards at the end of the function, we will add the new card at the end of the array. */
			free_card(cards[dupe_return]);
			for (int i = dupe_return; i < total_cards - 1; i++) cards[i] = cards[i + 1];
        	total_cards--;
		}
	}
	// allocate memory for new card and set the id and name
	CARD_T *card = malloc(sizeof(CARD_T));
	card->id = id;
	card->name = strdup(token);

	// skip comma
	stringp++;
	// get cost
	token = strsep(&stringp, ",");
	card->cost = atoi(token);

	char *text;
	// use strdup so for all scenarios so freeing memory is way easier
	// if string pointer points to a quote
	if (stringp == strstr(stringp, "\"")) {
		stringp++;
		char *double_quote = strstr(stringp, "\"\"");
		if (double_quote) {
			// while the double quote pointer doesn't point to a double quote followed by a comma
			while (double_quote != strstr(double_quote, "\","))
				double_quote++;
			int index = double_quote - stringp;
			text = strndup(stringp, index);
			stringp += index + 1;
		}
		else
			text = strdup(strsep(&stringp, "\""));
	}
	// string pointer must be pointing to a null field
	else
		text = strdup("");

	card->text = fix_text(text);

	stringp++;
	// get attack
	token = strsep(&stringp, ",");
	card->attack = atoi(token);
	
	// get health
	token = strsep(&stringp, ",");
	card->health = atoi(token);
	
	// Get type
	stringp++;
	token = strsep(&stringp, "\"");

	for (int i = 0; i < 5; i++) {
		if (strcasecmp(type_str[i], token) == 0) {
			card->type = i;
			break;
		}
	}
	
	// Get Class 
	stringp += 2;
	token = strsep(&stringp, "\"");
	
	for (int i = 0; i < 12; i++) {
		if (strcasecmp(class_str[i], token) == 0) {
			card->class = i;
			break;
		}
	}
	
	// Get Rarity
	stringp += 2;
	token = strsep(&stringp, "\"");
	
	for (int i = 0; i < 6; i++) {
		if (strcasecmp(rarity_str[i], token) == 0) {
			card->rarity = i;
			break;
		}
	}

	return card; // Char **

}

/*
 * Because getting the card class centered is such
 * a chore, you can have this function for free :)
 */
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
