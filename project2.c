// Kaylan Johnson <johnsk18>
// Xiao Jiang <jiangx5>
// Ruowen Qin <qinr>

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define FRAMES_PER_LINE 32
#define LINES 8
#define READY 0
#define RUNNING 1
#define BLOCKED 2
#define ARRIVING 3

typedef struct process { // struct for storing the data of a process
	char proc_id; // process id 
	int frames; // number of memory frames
	int arr_time; // arrival time
	int run_time; // run time
	int state; // state process is in
	int update_time; // time when state is completed
	int x; // first x posiiton for process segment in memory
	int y; // first y posiiton for process segment in memory
} process;

typedef struct memory {
	char** board; // memory board
	int frames_per_line; // number of frames per line
	int lines; // number of lines
} memory;

typedef struct position {
	int x; // x-coordinate
	int y; // y-coordinate
} position;

void error() { // outputs error
	perror("ERROR");
	exit(EXIT_FAILURE);
}

void debugPrintProcesses(process** processes, int n) {
	int i;
	for (i = 0; i < n; i++)	printf("%c %5d%3c%5d/%-5d%5d\n",(*processes)[i].proc_id,(*processes)[i].frames,' ',(*processes)[i].arr_time,(*processes)[i].run_time,(*processes)[i].update_time);
	printf("\n");
	fflush(stdout);	
}

int comparator(const void* a, const void* b) { // comparator to handle ties
	process p = *((process*) a);
	process q = *((process*) b);
	int diff = p.arr_time - q.arr_time;
	if (diff == 0) return p.proc_id - q.proc_id;
	return diff;
}

char** createBoard() { // creates board
	int i;
	char** board = (char**)calloc(LINES,sizeof(char*));
	for (i = 0; i < LINES; i++) {
		board[i] = (char*)calloc(FRAMES_PER_LINE+1,sizeof(char));
		board[i][FRAMES_PER_LINE] = '\0';
		memset(board[i],'.',FRAMES_PER_LINE);
	}
	return board;
}

void printBoard(char*** board) {
	int i;
	for (i = 0; i < FRAMES_PER_LINE; i++) printf("=");
	printf("\n");
	for (i = 0; i < LINES; i++) printf("%s\n",(*board)[i]);
	for (i = 0; i < FRAMES_PER_LINE; i++) printf("=");
	printf("\n");
	fflush(stdout);
}

position findBestFitPosition(char*** board, int frames) {
	char ** temp = *board;
	int i,j;
	int min_size = 257, size = 0, free_space = 0;
	position pos = {.x = 0, .y = 0};
	for (i = 0; i < LINES; i++) {
		for (j = 0; j < FRAMES_PER_LINE; j++) {
			if (temp[i][j] == '.') { // if space is empty
				++size;
				++free_space;
			} else { // if space is used
				if (size != 0) { // free space ends
					if (min_size > size && size >= frames) {
						min_size = size;
						pos = (position){.x = (((FRAMES_PER_LINE * i) + j - size) / FRAMES_PER_LINE), .y = (((FRAMES_PER_LINE * i) + j - size) % FRAMES_PER_LINE)}; // obtains the first free space position in free space segment
					} else size = 0; // moving through used memory
				}
			}
		}
	}
	if (min_size > size && size >= frames) return (position){.x = (((FRAMES_PER_LINE * (LINES - 1)) + FRAMES_PER_LINE - size) / FRAMES_PER_LINE), .y = (((FRAMES_PER_LINE * (LINES - 1)) + FRAMES_PER_LINE - size) % FRAMES_PER_LINE)}; // if very last free space is sufficient
	if (min_size != 257 && min_size >= frames) return pos; // if changed minimum size position meets criteria
	else {
		if (free_space >= frames) return (position){.x = -2, .y = -2}; // defragment memory
		return (position){.x = -1, .y = -1}; // no space left for process
	}
}

int defragmentation(char*** board, process** processes, int n, int t) { // defragments memory and returns amount of frames moved
	int i, j, k, first_space = 0, moves = 0, id_found = 1, size = 0;
	position free_pos = (position){.x = 0, .y = 0};
	char* ids = (char*)calloc(n,sizeof(char));
	for (i = 0; i < LINES; i++) { // defragments memory
		for (j = 0; j < FRAMES_PER_LINE; j++) {
			if (!first_space && (*board)[i][j] == '.') {
				first_space = 1;
				free_pos = (position){.x = i, .y = j};
			} else if (first_space && (*board)[i][j] != '.') {
				first_space = 0;
				(*board)[free_pos.x][free_pos.y] = (*board)[i][j];
				(*board)[i][j] = '.';
				i = free_pos.x;
				j = free_pos.y;
				++moves;
				id_found = 0;
				for (k = 0; k < n; k++) {
					if (ids[k] == (*board)[i][j]) {
						id_found = 1;
						break;
					}
				}
				if (!id_found) ids[size++] = (*board)[i][j];
			}
		}
	}
	printf("time %dms: Defragmentation complete (moved %d frames:",t,moves);
	for (k = 0; k < size; k++) printf(" %c%s",ids[k], k == size - 1 ? ")\n" : ",");
	fflush(stdout);
	free(ids);
	char last_id = '.';
	for (i = 0; i < LINES; i++) { // resets process staring positions, arrival times, and update times
		for (j = 0; j < FRAMES_PER_LINE; j++) {
			if ((*board)[i][j] != last_id) {
				for (k = 0; k < n; k++) {
					if ((*processes)[k].proc_id == (*board)[i][j]) {
						last_id = (*board)[i][j];
						(*processes)[k].x = i;
						(*processes)[k].y = j;
						(*processes)[k].arr_time += moves;
						(*processes)[k].update_time += moves;
						break;
					}
				}
			}
		}
	}
	return moves;
}

void markBoard(char*** board, position pos, char id, int frames) {
	int i, j, counter = 0, start = 1;
	for (i = start == 1 ? pos.x : 0; i < LINES && counter < frames; i++) {
		for (j = start == 1 ? pos.y : 0; j < FRAMES_PER_LINE && counter < frames; j++) {
			start = 0;
			(*board)[i][j] = id;
			counter++;
		}
	}
	printBoard(board);
}

process* fileParser(int* n, const char** arg1) { // parses file into process struct
	FILE* inputFile = NULL;
	inputFile = fopen(*arg1,"r");
	if (inputFile == NULL) {
		fprintf(stderr, "ERROR: Invalid input file format\n");
		exit(EXIT_FAILURE);
	}
	process* processes = (process*)calloc(26,sizeof(process));
	char* buffer = (char*)calloc(100,sizeof(char));
	memset(buffer,'\0',100);
	while (fgets(buffer,100,inputFile) != NULL) { // reads line from file, buffering 100 characters
		if (isalpha(buffer[0])) { // checks to see if line can be parsed
			int i, j, multiple_times = 0, len = strlen(buffer);
			char* buffer2 = (char*)calloc(len,sizeof(char));
			memset(buffer2,'\0',len);
			processes[*n].proc_id = buffer[0];
			for (i = 1; i < len; i++) if (isdigit(buffer[i])) break; // sets i to p_mem starting digit position
			for (j = 0; i < len; i++,j++) {
				if (isdigit(buffer[i])) buffer2[j] = buffer[i];
				else break;
			}
			processes[*n].frames = atoi(buffer2);
			processes[*n].state = ARRIVING;
			processes[*n].update_time = 0;
			do {
				memset(buffer2,'\0',len);
				for (++i; i < len; i++) if (isdigit(buffer[i])) break; // sets i to arr_time starting digit position
				if (i >= len) break;
				if (multiple_times) {
					(*n)++;
					processes[*n].proc_id = processes[*n - 1].proc_id;
					processes[*n].frames = processes[*n - 1].frames;
					processes[*n].state = ARRIVING;
					processes[*n].update_time = 0;
				}
				for (j = 0; j < len; i++,j++) {
					if (isdigit(buffer[i])) buffer2[j] = buffer[i];
					else {
						processes[*n].arr_time = atoi(buffer2);
						break;
					}
				}
				memset(buffer2,'\0',len);
				for (++i; i < len; i++) if (isdigit(buffer[i])) break; // sets i to run_time starting digit position
				for (j = 0; j < len; i++,j++) {
					if (isdigit(buffer[i])) buffer2[j] = buffer[i];
					else {
						processes[*n].run_time = atoi(buffer2);
						multiple_times = 1;
						break;
					}
				}
			} while (i < len);
			free(buffer2);
			(*n)++;
		}
	}
	processes = (process*)realloc(processes, (*n) * (sizeof(process)));
	#ifdef DEBUG_MODE
		printf("\nprocesses parsed: %d\n",*n);
		debugPrintProcesses(&processes,*n);
		qsort(processes,*n,sizeof(process),comparator);
		printf("\nsorted processes parsed: %d\n",*n);
		debugPrintProcesses(&processes,*n);
	#endif
	fclose(inputFile);
	free(buffer);
	return processes;
}

void best_fit(process** parsed_processes, int n, int t_memmove) {
	int i, j, t = 0, terminated = 0;
	process* processes = (process*)calloc(n,sizeof(process));
	memcpy(&processes,parsed_processes,sizeof(process*));
	#ifdef DEBUG_MODE
		printf("\nprocesses copied: %d\n",n);
		debugPrintProcesses(&processes,n);
	#endif
	memory* mem = (memory*)calloc(1,sizeof(memory));
	mem->board = createBoard();
	printf("time %dms: Simulator started (Contiguous -- Best-Fit)\n",t);
	fflush(stdout);
	while (terminated < n) {
		for (i = 0; i < n - terminated; i++) { // loop for processes arriving and entering the CPU
			if (processes[i].update_time == t && processes[i].state == RUNNING) {
				printf("time %dms: Process %c removed:\n",t,processes[i].proc_id);
				fflush(stdout);
				position pos = (position){.x = processes[i].x, .y = processes[i].y};
				markBoard(&(mem->board),pos,'.',processes[i].frames);
				#ifdef DEBUG_MODE
					printf("\ntime %dms: processes before remove: %d\n",t,n - terminated);
					debugPrintProcesses(&processes,n - terminated);
				#endif
				for (j = i; j < n - terminated - 1; j++) processes[j] = processes[j+1];
				--i;
				++terminated;
				#ifdef DEBUG_MODE
					printf("\ntime %dms: processes after remove: %d\n",t,n - terminated);
					debugPrintProcesses(&processes,n - terminated);
				#endif
				continue;
			}
			if (processes[i].arr_time == t && processes[i].state == ARRIVING) { // process has arrived 
				printf("time %dms: Process %c arrived (requires %d frames)\n",t,processes[i].proc_id,processes[i].frames);
				fflush(stdout);
				position pos = findBestFitPosition(&(mem->board),processes[i].frames);
				if (pos.x == -1 && pos.y == -1) { // case where there is no space for process to take
					printf("time %dms: Cannot place process %c -- skipped!\n",t,processes[i].proc_id);
					fflush(stdout);
					#ifdef DEBUG_MODE
						printf("\ntime %dms: processes before remove: %d\n",t,n - terminated);
						debugPrintProcesses(&processes,n - terminated);
					#endif
					for (j = i; j < n - terminated - 1; j++) processes[j] = processes[j+1];
					--i;
					++terminated;
					#ifdef DEBUG_MODE
						printf("\ntime %dms: processes after remove: %d\n",t,n - terminated);
						debugPrintProcesses(&processes,n - terminated);
					#endif
					continue;
				} else { // process can be placed without defragmentation
					if (pos.x == -2 && pos.y == -2) { // case where if memory is defragmented, then the process can enter
						printf("time %dms: Cannot place process %c -- starting defragmentation\n",t,processes[i].proc_id);
						fflush(stdout);
						defragmentation(&(mem->board),&processes,n-terminated,t);
						#ifdef DEBUG_MODE
							printf("\nDefragmentation:\n");
							printBoard(&(mem->board));
							printf("\n");
						#endif
						pos = findBestFitPosition(&(mem->board),processes[i].frames);
					}
					printf("time %dms: Placed process %c:\n",t,processes[i].proc_id);
					fflush(stdout);
					processes[i].x = pos.x;
					processes[i].y = pos.y;
					processes[i].state = RUNNING;
					processes[i].update_time = t + processes[i].run_time;
					markBoard(&(mem->board),pos,processes[i].proc_id,processes[i].frames);
					#ifdef DEBUG_MODE
						printf("\ntime %dms: processes updated:\n",t);
						debugPrintProcesses(&processes,n - terminated);
					#endif
				}
			}
		}
		if (terminated == n) break; // ends simulation when all processes have been terminated
		t++;
	}
	printf("time %dms: Simulator ended (Contiguous -- Best-Fit)\n",t);
	fflush(stdout);
}

int main(int argc, char const *argv[]) {
	if (argc < 2) { // error handling for command-line arguments
		fprintf(stderr, "ERROR: Invalid arugment(s)\nUSAGE: ./a.out <input-file>\n");
		exit(EXIT_FAILURE);
	}
	int n = 0; // the number of processes to simulate
	int t_memmove = 1; // time required to move one frame of memory (in milliseconds)
	process* processes = fileParser(&n, &argv[1]);
	best_fit(&processes,n,t_memmove);
	free(processes);
	return EXIT_SUCCESS;
}