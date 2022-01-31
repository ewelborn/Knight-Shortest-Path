#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define DEBUG
#undef DEBUG

enum positionState {
	NOT_VISITED, // Free for exploration!
	CONSIDERING_NEXT_STAGE, // Marked for consideration on next iteration
	CONSIDERING, // Marked for consideration on this iteration
	VISITED, // Ignore
} typedef PositionState;

struct position {
	uint8_t rank;
	uint8_t file;
} typedef Position;

// Reverses a given string
void reverse(char* s)
{
	// If the string is an even number of characters, then we want
	// to swap every pair of characters like so (ex. hello!):
	// [!]ello[h] -> ![o]ll[e]H -> !o[l][l]eh
	// If the string is an odd number of characters, then we want to
	// avoid the character in the middle.

	int length = strlen(s);

	for (int i = 0; i < (length / 2); i++) {
		char hold = s[i];
		s[i] = s[length - 1 - i]; // -1 so we don't accidently swap the \0 byte :)
		s[length - 1 - i] = hold;
	}
}

// Takes in a 2 character position on the board and returns
// the rank and file of the position. Ex. a5 is rank 0, file 4
// (null terminator optional for coordinate string)
// This function returns true if it succeeds, false otherwise
bool coordinateToRankAndFile(char* coordinate, Position* position)
{
	if (coordinate[0] >= 'a' && coordinate[0] <= 'h') {
		position->file = coordinate[0] - 'a';
	} 
	else if (coordinate[0] >= 'A' && coordinate[0] <= 'H') {
		position->file = coordinate[0] - 'A';
	}
	else {
		return false;
	}

	if (coordinate[1] >= '1' && coordinate[1] <= '8') {
		position->rank = coordinate[1] - '1';
	}
	else {
		return false;
	}

	return true;
}

// This function returns the shortest path for a knight to travel
// from the given start position to the given end position of 
// an 8x8 chessboard.
// start and end is a 2 character position on the board, ex. a5 (null terminator optional)
// path is a string that contains the board positions that the knight
// must hop to in order to reach the end. The path includes the start
// position and end position. The path string is null terminated.
// The path string must be provided by the caller.
// This function returns the number of moves necessary for the knight to
// reach its goal.
uint8_t findShortestPathForKnight(char* start, char* end, char* path)
{
	// These are variables used later on in SEARCH_COMPLETE
	int moves = 0;
	Position currentP;
	currentP.rank = 0;
	currentP.file = 0;

	// Convert the coordinates into position structs so they're
	// easier to do math on.
	Position startP;
	Position endP;
	coordinateToRankAndFile(start, &startP);
	coordinateToRankAndFile(end, &endP);

	// *** Start breadth-first search

	// The following array keeps track of which positions have been searched
	PositionState board[8][8] = { NOT_VISITED };
	// The following array keeps track of the previous position that was used to reach
	// the current position
	Position boardPath[8][8];

	board[startP.rank][startP.file] = CONSIDERING;

	while (true) {
		for (int rank = 0; rank < 8; rank++) {
			for (int file = 0; file < 8; file++) {
				if (board[rank][file] == CONSIDERING) {

					board[rank][file] = VISITED;
					#ifdef DEBUG
					printf("%c%c visited\n", file+'a', rank+'1');
					#endif

					if (rank == endP.rank && file == endP.file) {
						// We're done!
						currentP.rank = endP.rank;
						currentP.file = endP.file;
						goto SEARCH_COMPLETE;
					}

					// Generate all knight moves and check if the move is valid on the board,
					// (i.e. not out of bounds) if it is valid, then check if the position has
					// been considered yet, if not, then flag it for consideration and mark
					// the current position as the path that was used to reach it.
					for (int i = 0; i < 8; i++) {
						int dRank = (i & 0b100) == 0b100 ? 2 : 1;
						int dFile = (i & 0b100) == 0b100 ? 1 : 2;
						if ((i & 0b1) == 0b1) {
							dRank = -dRank;
						}
						if ((i & 0b10) == 0b10) {
							dFile = -dFile;
						}
						int nRank = rank + dRank;
						int nFile = file + dFile;
						if (nRank < 0 || nRank > 7) { continue; }
						if (nFile < 0 || nFile > 7) { continue; }

						if (board[nRank][nFile] == NOT_VISITED) {
							board[nRank][nFile] = CONSIDERING_NEXT_STAGE;
							boardPath[nRank][nFile].rank = rank;
							boardPath[nRank][nFile].file = file;
							#ifdef DEBUG
							printf("  considering %c%c\n", nFile + 'a', nRank + '1');
							#endif
						}

					}
				}
			}
		}

		// Flip the flags for all CONSIDERING_NEXT_STAGE positions, because
		// we're now in the next stage, and we want to consider them :)
		for (int rank = 0; rank < 8; rank++) {
			for (int file = 0; file < 8; file++) {
				if (board[rank][file] == CONSIDERING_NEXT_STAGE) {
					board[rank][file] = CONSIDERING;
				}
			}
		}
	}

SEARCH_COMPLETE:
	// Go backwards from the end position to find the path used, and
	// build the path string in reverse
	// e.g. if the true path is a1 -> b3 -> d4, then the generated path string
	// will be 4d3b1a. Once we have this string, we can reverse it and exit the function.
	while (true) {
		if (currentP.file == startP.file && currentP.rank == startP.rank) {
			// We reached the start position!

			path[(moves * 2) + 0] = startP.rank + '1';
			path[(moves * 2) + 1] = startP.file + 'a';
			path[(moves * 2) + 2] = '\0';

			reverse(path);

			return moves;
		}

		path[(moves * 2) + 0] = currentP.rank + '1';
		path[(moves * 2) + 1] = currentP.file + 'a';
		moves++;

		int currentRank = currentP.rank;
		int currentFile = currentP.file;
		currentP.rank = boardPath[currentRank][currentFile].rank;
		currentP.file = boardPath[currentRank][currentFile].file;
	}
}

// Used to avoid garbage added by windows terminal when pressing enter
char getCharIgnoreGarbage() {
	char result = 0;
	while (true) {
		result = getchar();
		if (result == 10) {
			continue;
		}
		return result;
	}
}

int main(void)
{
	char path[200] = { 0 };
	char start[3] = { 0 };
	char end[3] = { 0 };

	printf("Start coordinate and end coordinate (ex. a1h7): ");
	
	start[0] = getCharIgnoreGarbage();
	start[1] = getCharIgnoreGarbage();
	start[2] = '\0';
	end[0] = getCharIgnoreGarbage();
	end[1] = getCharIgnoreGarbage();
	end[2] = '\0';

	int moves = findShortestPathForKnight(start, end, path);
	printf("Shortest path:");
	printf("\n\tMoves: %d\n\tPath: %s\n", moves, path);
}