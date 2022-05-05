#include <fcntl.h>
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFSIZE 1024

typedef struct {
	char word[42];
	unsigned count;
} WORD_T;

WORD_T *words = NULL;
size_t total_words = 0;

void print_and_free(WORD_T*, size_t, char*);
int comparator(const void *a, const void *b) {
	char *a_key = (char *) a;
	WORD_T *b_element = (WORD_T *) b;
	return strcmp(a_key, b_element->word);
}

int main(int argc, char **argv) {
	if (argc != 2) return -1;

	char *infile = argv[1];
	// TODO: open the file O_RDONLY
	int fd = open(infile, O_RDONLY);
	if (fd == -1){
		perror("Open Error");
		return -2;
	}
	
	// holds 1024 bytes of text at a time
	char buffer[BUFSIZE];
	// pointer that points to start of buffer
	char *str_pointer = buffer;
	// holds num of bytes read from read()
	int bytes = -1;
	int read_offset = 0;

	// TODO: repeatedly call `read()` into a buffer of
	//       size BUFSIZE. Split the text in the buffer
	//       on newlines and spaces. For each token:
	//       search the `words` array to see if that
	//       word has already been added and if so
	//       increment the count. Otherwise add a new
	//       WORD_T struct to the end of the array of
	//       structs `words` and set the fields accordingly.

	while (bytes != 0){
		// read file descriptor into buffer, store num of bytes read into bytes var
		bytes = read(fd, str_pointer + read_offset, BUFSIZE - 1 - read_offset);
		// last time in loop bytes read was 735, now if it's zero, break out of loop
		if (bytes == 0) break;
		if (bytes < 0){
			perror("Read Error");
			break;
		}
		// null terminate the buffer at the number of bytes read + the offset
		buffer[bytes + read_offset] = '\0';
		while (str_pointer != NULL){
			char *token = strsep(&str_pointer, "\n ");
			if (str_pointer == NULL){
				// TODO: its possible that a word is split between
				//       one fill of the buffer and the next. You must
				//       move the last word at the end of the buffer to
				//       the beginning of the buffer and then fill
				//       the buffer from that point!
				read_offset = strlen(token);
				// copies the string token, stores at the beginning of the buffer, and size is string length of token + 1, +1 to null terminate.
				strncpy(&buffer[0], token, read_offset + 1);
				token = NULL;
				break;
			}
			if (strlen(token) > 0){
				// find in words struct
				WORD_T *result = lfind(token, words, &total_words, sizeof(WORD_T), comparator);
				// if result was found increment the count of the word
				if (result != NULL){
					result->count++;
				}
				else{
					// reallocate memory for words array
					words = realloc(words, sizeof(WORD_T) * (total_words + 1));
					// copy the word into words array at total length
					strcpy(words[total_words].word, token);
					// set the count
					words[total_words].count = 1;
					// increment total words
					total_words++;
				}
			}
		}
		// reinitialize pointer
		str_pointer = buffer;
	}
	print_and_free(words, total_words, infile);
	// TODO: close the file
	close(fd);
	return 0;
}

void print_and_free(WORD_T *words, size_t total_words, char *infile) {
	int sum = 0;
	for (int i = 0; i < total_words; ++i) {
		if (words[i].count > 1)
			printf("%s: %u\n", words[i].word, words[i].count);
		sum += words[i].count;
	}
	printf("\n%d %s\n", sum, infile);

	free(words);
}