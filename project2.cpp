// Kaylan Johnson <johnsk18>
// Xiao Jiang <jiangx5>
// Ruowen Qin <qinr>

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define FRAMES_PER_LINE 32
#define LINES 8
#define ARRIVING 0
#define RUNNING 1

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
	char **board; // memory board
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

void debugPrintProcesses(process **processes, int n) {
	int i;
	for (i = 0; i < n; i++) printf("%c %5d%3c%5d/%-5d%5d\n", (*processes)[i].proc_id, (*processes)[i].frames, ' ', (*processes)[i].arr_time, (*processes)[i].run_time, (*processes)[i].update_time);
	printf("\n");
	fflush(stdout);
}

int comparator(const void *a, const void *b) { // comparator to handle ties
	process p = *((process *) a);
	process q = *((process *) b);
	int diff = p.arr_time - q.arr_time;
	if (diff == 0) return p.proc_id - q.proc_id;
	return diff;
}

char **createBoard() { // creates board
	int i;
	char **board = (char **) calloc(LINES, sizeof(char *));
	for (i = 0; i < LINES; i++) {
		board[i] = (char *) calloc(FRAMES_PER_LINE + 1, sizeof(char));
		board[i][FRAMES_PER_LINE] = '\0';
		memset(board[i], '.', FRAMES_PER_LINE);
	}
	return board;
}

void freeBoard(char ***board) { // frees the board
	int i;
	for (i = 0; i < LINES; i++) {
		free((*board)[i]);
		(*board)[i] = NULL;
	}
	free(*board);
	(*board) = NULL;
}

void printBoard(char ***board) {
	int i;
	for (i = 0; i < FRAMES_PER_LINE; i++) printf("=");
	printf("\n");
	for (i = 0; i < LINES; i++) printf("%s\n", (*board)[i]);
	for (i = 0; i < FRAMES_PER_LINE; i++) printf("=");
	printf("\n");
	fflush(stdout);
}

void resize(process **processes, int *capacity) { // increases array size by 100
	*capacity += 100;
	*processes = static_cast<process *>(realloc(*processes, (*capacity) * (sizeof(process))));
}

/*
when merging code:
update palceprocess() return value to be position, needed for my algo. 
after calling placeprocess(), add lines of last_x && last_y
*/
position findNextFitPosition(char** board, int frames, process* processes, int last_x, int last_y) { // TODO: implement code for next-fit, returning the starting position of the first free partition after the end of the last placed process
	int start_x=last_x; 
	int start_y=last_y;/*
	if (start_y+1==FRAMES_PER_LINE){
		start_y=0;
		if (start_x+1==LINES){
			start_x=0;
		}
		else{
			start_x=start_x+1;
		}
	}
	else{
		start_x=start_x+1;
		start_y=start_y+1;
	}*/
	#if D
	printf("last_x %d,last_y %d\n",last_x,last_y);
	#endif 
	int length=0;
	int space=0;
	//move end_x, end_y through the board while exploring
	for (int i = last_x; i < LINES; ++i) {
		for (int j = 0; j < FRAMES_PER_LINE; ++j) {
			if (i==last_x && j<last_y){continue;}
			if (board[i][j]!='.'){
				length=0;
				start_x=i;
				start_y=j;				
			} else {//if this is a good frame
				if (length==0) {//if this is the first good consecutive frame
					start_x=i;
					start_y=j;
					#if D
						printf("reset start   x[%d] y[%d]\n",start_x,start_y);
						fflush(stdout);
					#endif	
				}
				length++;
				space++;
			}
			if (length==frames) {
				#if D
					printf("find postiions  x[%d] y[%d]\n",start_x,start_y);
					fflush(stdout);
				#endif
				return (position){.x=start_x,.y=start_y};
			}
		}
	}
	#if D
		printf("finished till end\n");
	#endif
	start_x=0;
	start_y=0;
	length=0;
	for (int i = 0; i < LINES; ++i) {
		for (int j = 0; j < FRAMES_PER_LINE; ++j) {
			if (i==last_x && j==last_y) {
				if (space>=frames) return (position){.x=-2, .y=-2};
				else return (position) {.x=-1,.y=-1};
			}
			if (board[i][j]!='.'){
				length=0;
				start_x=i;
				start_y=j;
			} else {
				if (length==0) {
					start_x=i;
					start_y=j;
				}
				length++;
				space++;
			}
			if (length==frames) return (position){.x=start_x,.y=start_y};
		}
	}
	return (position){.x = -1, .y = -1}; // never supposed to be hit, but for compilation resolvement
}

position findBestFitPosition(char ***board, int frames) { // best-fit algorithm for returning the starting position of the smallest fitting free partition
	char **temp = *board;
	int i, j;
	int min_size = (LINES * FRAMES_PER_LINE) + 1, size = 0, free_space = 0;
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
						pos = (position) {.x = (((FRAMES_PER_LINE * i) + j - size) / FRAMES_PER_LINE), .y = (((FRAMES_PER_LINE * i) + j - size) % FRAMES_PER_LINE)}; // obtains the first free space position in free space segment
					} else size = 0; // moving through used memory
				}
			}
		}
	}
	if (min_size > size && size >= frames) return (position) {.x = (((FRAMES_PER_LINE * (LINES - 1)) + FRAMES_PER_LINE - size) / FRAMES_PER_LINE), .y = (((FRAMES_PER_LINE * (LINES - 1)) + FRAMES_PER_LINE - size) % FRAMES_PER_LINE)}; // if very last free space is sufficient
	if (min_size != ((LINES * FRAMES_PER_LINE) + 1) && min_size >= frames) return pos; // if changed minimum size position meets criteria
	else {
		if (free_space >= frames) return (position) {.x = -2, .y = -2}; // defragment memory
		return (position) {.x = -1, .y = -1}; // no space left for process
	}
}

position findWorstFitPosition(char ***board, int frames) {  // worst-fit algorithm for returning the starting position of the largest fitting free partition
	char **temp = *board;
	int i, j;
	int max_size = 0, size = 0, free_space = 0;
	position pos = {.x = 0, .y = 0};
	for (i = 0; i < LINES; i++) {
		for (j = 0; j < FRAMES_PER_LINE; j++) {
			if (temp[i][j] == '.') { // if space is empty
				++size;
				++free_space;
			} else { // if space is used
				if (size != 0) { // free space ends
					if (max_size < size && size >= frames) {
						max_size = size;
						pos = (position) {.x = (((FRAMES_PER_LINE * i) + j - size) / FRAMES_PER_LINE), .y = (((FRAMES_PER_LINE * i) + j - size) %FRAMES_PER_LINE)}; // obtains the first free space position in free space segment
					} else size = 0; // moving through used memory
				}
			}
		}
	}
	if (max_size < size && size >= frames) return (position) {.x = (((FRAMES_PER_LINE * (LINES - 1)) + FRAMES_PER_LINE - size) / FRAMES_PER_LINE), .y = (((FRAMES_PER_LINE * (LINES - 1)) + FRAMES_PER_LINE - size) % FRAMES_PER_LINE)}; // if very last free space is sufficient
	if (max_size != 0 && max_size >= frames) return pos; // if changed minimum size position meets criteria
	else {
		if (free_space >= frames) return (position) {.x = -2, .y = -2}; // defragment memory
		return (position) {.x = -1, .y = -1}; // no space left for process
	}
}

void defragmentation(char ***board, process **processes, int n, int *t, int t_memmove) { // defragments memory
	int i, j, k, first_space = 0, moves = 0, id_found = 1, size = 0;
	position free_pos = (position) {.x = 0, .y = 0};
	char *ids = (char *) calloc(n, sizeof(char));
	for (i = 0; i < LINES; i++) { // defragments memory
		for (j = 0; j < FRAMES_PER_LINE; j++) {
			if (!first_space && (*board)[i][j] == '.') {
				first_space = 1;
				free_pos = (position) {.x = i, .y = j};
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
	*t += (moves * t_memmove);
	printf("time %dms: Defragmentation complete (moved %d frames:", *t, moves);
	for (k = 0; k < size; k++) printf(" %c%s", ids[k], k == size - 1 ? ")\n" : ",");
	fflush(stdout);
	printBoard(board);
	free(ids);
	char last_id = '.';
	for (i = 0; i < LINES; i++) { // resets all moved process staring positions
		for (j = 0; j < FRAMES_PER_LINE; j++) {
			if ((*board)[i][j] != last_id) {
				for (k = 0; k < n; k++) {
					if ((*processes)[k].proc_id == (*board)[i][j]) {
						last_id = (*board)[i][j];
						(*processes)[k].x = i;
						(*processes)[k].y = j;
						break;
					}
				}
			}
		}
	}
	for (k = 0; k < n; k++) { // resets all process arrival times and update times
		(*processes)[k].arr_time += (moves * t_memmove);
		if ((*processes)[k].state == RUNNING) (*processes)[k].update_time += (moves * t_memmove);
	}
}

void placeProcess(char ***board, position pos, char id, int frames) { // places process in memory board
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

process *fileParser(int *n, const char **arg1) { // parses file into process struct array
	FILE *inputFile = NULL;
	inputFile = fopen(*arg1, "r");
	if (inputFile == NULL) {
		fprintf(stderr, "ERROR: Invalid input file format\n");
		exit(EXIT_FAILURE);
	}
	int capacity = 100;
	process *processes = (process *) calloc(capacity, sizeof(process));
	char *buffer = (char *) calloc(100, sizeof(char));
	memset(buffer, '\0', 100);
	while (fgets(buffer, 100, inputFile) != NULL) { // reads line from file, buffering 100 characters
		if (isalpha(buffer[0])) { // checks to see if line can be parsed
			if (*n >= capacity) resize(&processes, &capacity);
			int i, j, multiple_times = 0, len = strlen(buffer);
			char *buffer2 = (char *) calloc(len, sizeof(char));
			memset(buffer2, '\0', len);
			processes[*n].proc_id = buffer[0];
			for (i = 1; i < len; i++) if (isdigit(buffer[i])) break; // sets i to p_mem starting digit position
			for (j = 0; i < len; i++, j++) {
				if (isdigit(buffer[i])) buffer2[j] = buffer[i];
				else break;
			}
			processes[*n].frames = atoi(buffer2);
			processes[*n].state = ARRIVING;
			processes[*n].update_time = 0;
			do {
				memset(buffer2, '\0', len);
				for (++i; i < len; i++) if (isdigit(buffer[i])) break; // sets i to arr_time starting digit position
				if (i >= len) break;
				if (multiple_times) {
					(*n)++;
					if (*n >= capacity) resize(&processes, &capacity);
					processes[*n].proc_id = processes[*n - 1].proc_id;
					processes[*n].frames = processes[*n - 1].frames;
					processes[*n].state = ARRIVING;
					processes[*n].update_time = 0;
				}
				for (j = 0; j < len; i++, j++) {
					if (isdigit(buffer[i])) buffer2[j] = buffer[i];
					else {
						processes[*n].arr_time = atoi(buffer2);
						break;
					}
				}
				memset(buffer2, '\0', len);
				for (++i; i < len; i++) if (isdigit(buffer[i])) break; // sets i to run_time starting digit position
				for (j = 0; j < len; i++, j++) {
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
	if (*n < capacity) processes = (process *) realloc(processes, (*n) * (sizeof(process)));
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

void contiguous(process **parsed_processes, int n, int t_memmove) { // simulates contiguous memory management
	int alg = 0; // TODO: set to 0 when next-fit function added
	int last_x=0, last_y=0;
	for (; alg < 3; alg++) { // loops though placement simulations. alg = 0 = next-fit, alg = 1 = best-fit, alg = 2 = worst-fit
		int i, j, t = 0, terminated = 0;
		process *processes = (process *) calloc(n, sizeof(process));
		memcpy(processes, *parsed_processes, sizeof(process) * n);
		#ifdef DEBUG_MODE
			printf("\nprocesses copied: %d\n",n);
			debugPrintProcesses(&processes,n);
		#endif
		memory *mem = (memory *) calloc(1, sizeof(memory));
		mem->board = createBoard();
		printf("time %dms: Simulator started (Contiguous -- %s)\n",t, alg == 0 ? "Next-Fit" : alg == 1 ? "Best-Fit" : "Worst-Fit");
		fflush(stdout);
		while (terminated < n) { // begins simulation
			for (i = 0; i < n - terminated; i++) { // loop for processes leaving
				if (processes[i].update_time == t && processes[i].state == RUNNING) {
					printf("time %dms: Process %c removed:\n", t, processes[i].proc_id);
					fflush(stdout);
					position pos = (position) {.x = processes[i].x, .y = processes[i].y};
					placeProcess(&(mem->board), pos, '.', processes[i].frames);
					#ifdef DEBUG_MODE
						printf("\ntime %dms: processes before remove: %d\n",t,n - terminated);
						debugPrintProcesses(&processes,n - terminated);
					#endif
					for (j = i; j < n - terminated - 1; j++) processes[j] = processes[j + 1];
					--i;
					++terminated;
					#ifdef DEBUG_MODE
						printf("\ntime %dms: processes after remove: %d\n",t,n - terminated);
						debugPrintProcesses(&processes,n - terminated);
					#endif
					continue;
				}
			}
			for (i = 0; i < n - terminated; i++) { // loop for processes arriving
				if (processes[i].arr_time == t && processes[i].state == ARRIVING) { // process has arrived
					printf("time %dms: Process %c arrived (requires %d frames)\n", t, processes[i].proc_id,
						   processes[i].frames);
					fflush(stdout);
					position pos;
					if (alg == 0) pos = findNextFitPosition(mem->board,processes[i].frames,processes, last_x, last_y);
					else if (alg == 1) pos = findBestFitPosition(&(mem->board), processes[i].frames);
					else pos = findWorstFitPosition(&(mem->board), processes[i].frames);
					if (pos.x == -1 && pos.y == -1) { // case where there is no space for process to take
						printf("time %dms: Cannot place process %c -- skipped!\n", t, processes[i].proc_id);
						fflush(stdout);
						#ifdef DEBUG_MODE
							printf("\ntime %dms: processes before remove: %d\n",t,n - terminated);
							debugPrintProcesses(&processes,n - terminated);
						#endif
						for (j = i; j < n - terminated - 1; j++) processes[j] = processes[j + 1];
						--i;
						++terminated;
						#ifdef DEBUG_MODE
							printf("\ntime %dms: processes after remove: %d\n",t,n - terminated);
							debugPrintProcesses(&processes,n - terminated);
						#endif
						continue;
					} else { // process can be placed without defragmentation
						if (pos.x == -2 &&
							pos.y == -2) { // case where if memory is defragmented, then the process can enter
							printf("time %dms: Cannot place process %c -- starting defragmentation\n", t, processes[i].proc_id);
							fflush(stdout);
							defragmentation(&(mem->board), &processes, n - terminated, &t, t_memmove);
							if (alg == 0) pos = findNextFitPosition(mem->board,processes[i].frames,processes, 0,0);
							else if (alg == 1) pos = findBestFitPosition(&(mem->board), processes[i].frames);
							else pos = findWorstFitPosition(&(mem->board), processes[i].frames);
						}
						printf("time %dms: Placed process %c:\n", t, processes[i].proc_id);
						fflush(stdout);
						processes[i].x = pos.x;
						processes[i].y = pos.y;
						processes[i].state = RUNNING;
						processes[i].update_time = t + processes[i].run_time;
						placeProcess(&(mem->board), pos, processes[i].proc_id, processes[i].frames);
						#ifdef DEBUG_MODE
							printf("\ntime %dms: processes updated:\n",t);
							debugPrintProcesses(&processes,n - terminated);
						#endif
						last_x=pos.x+processes[i].frames/32;
						last_y=pos.y+processes[i].frames%32;
						if (last_y>31){
							last_y=last_y-32;
							last_x=last_x+1;
						}
					}
				}
			}
			if (terminated == n) break; // ends simulation when all processes have been terminated
			t++;
		}
		printf("time %dms: Simulator ended (Contiguous -- %s)\n\n", t, alg == 0 ? "Next-Fit" : alg == 1 ? "Best-Fit" : "Worst-Fit");
		fflush(stdout);
		free(processes);
		freeBoard(&(mem->board));
		free(mem);
	}
}

//
// Created by Ruowen Qin on 2018/4/29.
//

position findPlace(char ***board) { // find the first empty places
	position pos = {.x = -1, .y =-1};
	int i, j;
	for (i = 0; i < LINES; ++i) {
		for (j = 0; j < FRAMES_PER_LINE; ++j) {
			if ((*board)[i][j] == '.') {
				pos.x = j;
				pos.y = i;
				return pos;
			}
		}
	}
	return pos;
}

void processLeave(char ***board, position pos, char id, int frames) {
	for (int i = pos.y; i < LINES; ++i) {
		for (int j = 0; j < FRAMES_PER_LINE; ++j) {
			if ((*board)[i][j] == id && frames != 0) {
				(*board)[i][j] = '.';
				frames--;
			}
		}
	}
	printBoard(board);

}

void placeProcess_Non(char ***board, position pos, char id, int frames) { // places process in memory board
	int i,j;
	for (i = pos.y; i < LINES; ++i) {
		for (j = 0; j < FRAMES_PER_LINE; ++j) {
			if ((*board)[i][j] == '.' && frames != 0) {
				(*board)[i][j] = id;
				frames--;
			}
		}
	}
	printBoard(board);
}

void swap(char **queue, int x, int y) {
	char temp;
	char temp2;
	temp = (*queue)[x];
	temp2 = (*queue)[y];
	(*queue)[x] = temp2;
	(*queue)[y] = temp;
}

void queueSort(char **queue, int low, int high) {
	int i, last;
	if (low < high) {
		last = low;
		for (i = low + 1; i < high; ++i) if ((*queue)[i] < (*queue)[low]) swap(queue, ++last, i);
		swap(queue, last, low);
		queueSort(queue, low, last - 1);
		queueSort(queue, last + 1, high);
	}
}

int partition(char **queue, int low, int high) {
	char key;
	key = (*queue)[low];
	while (low < high) {
		while (low < high && (*queue)[high] >= key) high--;
		if (low < high) (*queue)[low++] = (*queue)[high];
		while (low < high && (*queue)[low] <= key) low++;
		if (low < high) (*queue)[high--] = (*queue)[low];
		(*queue)[low] = key;
	}
	return low;
}

void quick_sort(char **queue, int start, int end) {
	int pos;
	if (start < end) {
		pos = partition(queue, start, end);
		quick_sort(queue, start, pos - 1);
		quick_sort(queue, pos + 1, end);
	}
}

int findInList(char *list, char item, int length) {
	int i;
	for (i = 0; i < length; ++i) if (list[i] == item) return 1;
	return 0;
}

void pageTablePrint(char ***board, int n) {
	int i,j,k,count = 0;
	printf("PAGE TABLE [page,frame]:\n");
	char *idList = (char *) (calloc((size_t) n, sizeof(char)));
	for (i = 0; i < LINES; ++i) {
		for (j = 0; j < FRAMES_PER_LINE; ++j) {
			if ((*board)[i][j] != '.') if (!findInList(idList, (*board)[i][j], count)) idList[count++] = (*board)[i][j];
		}
	}
	quick_sort(&idList, 0, count - 1);
	for (k = 0; k < count; ++k) {
		int count_ID = 0;
		int count_10 = 0;
		printf("%c: ", idList[k]);
		for (i = 0; i < LINES; ++i) {
			for (j = 0; j < FRAMES_PER_LINE; ++j) {
				if ((*board)[i][j] == idList[k]) {
					count_10++;
					printf("[%d,%d] ", count_ID, i * FRAMES_PER_LINE + j);
					fflush(stdout);
					count_ID++;
					if (count_10 == 10) {
						count_10 = 0;
						printf("\n");
						fflush(stdout);
					}
				}
			}
		}
		printf("\n");
		fflush(stdout);
	}
}

int findFreePlace(char ***board) {
	int i,j,count = 0;
	for (i = 0; i < LINES; ++i) {
		for (j = 0; j < FRAMES_PER_LINE; ++j) {
			if ((*board)[i][j] == '.') count++;
		}
	}
	return count;
}

void noncontiguous(process **parsed_processes, int n) { // simulates non-contiguous memory managment
	process *processes = (process *) calloc((size_t) n, sizeof(process));
	memcpy(processes, *parsed_processes, sizeof(process) * n);
	int freePlace = 0;
	int i, j, t = 0, terminated = 0;
	#ifdef DEBUG_MODE
		printf("\nprocesses copied: %d\n",n);
		debugPrintProcesses(&processes,n);
	#endif
	memory *mem = (memory *) calloc(1, sizeof(memory));
	mem->board = createBoard();
	printf("time %dms: Simulator started (Non-contiguous)\n", t);
	while (terminated < n) {// begins simulation
		for (i = 0; i < n - terminated; i++) { // loop for processes leaving
			if (processes[i].update_time == t && processes[i].state == RUNNING) {
				printf("time %dms: Process %c removed:\n", t, processes[i].proc_id);
				fflush(stdout);
				position pos = (position) {.x = processes[i].x, .y = processes[i].y};
				processLeave(&(mem->board), pos, processes[i].proc_id, processes[i].frames);
				#ifdef DEBUG_MODE
					printf("\ntime %dms: processes before remove: %d\n",t,n - terminated);
					debugPrintProcesses(&processes,n - terminated);
				#endif
				for (j = i; j < n - terminated - 1; j++) processes[j] = processes[j + 1];
				--i;
				++terminated;
				#ifdef DEBUG_MODE
					printf("\ntime %dms: processes after remove: %d\n",t,n - terminated);
					debugPrintProcesses(&processes,n - terminated);
				#endif
				pageTablePrint(&(mem->board), n);
				continue;
			}
		}

		for (i = 0; i < n - terminated; i++) { // loop for processes arriving
			if (processes[i].arr_time == t && processes[i].state == ARRIVING) { // process has arrived
				printf("time %dms: Process %c arrived (requires %d frames)\n", t, processes[i].proc_id,
					   processes[i].frames);
				fflush(stdout);
				position pos;
				fflush(stdout);
				pos = findPlace(&(mem->board));
				freePlace = findFreePlace(&(mem->board));
				if ((pos.x == -1 && pos.y == -1) || (freePlace - processes[i].frames) < 0) {
					printf("time %dms: Cannot place process %c -- skipped!\n", t, processes[i].proc_id);
					fflush(stdout);
					#ifdef DEBUG_MODE
						printf("\ntime %dms: processes before remove: %d\n",t,n - terminated);
						debugPrintProcesses(&processes,n - terminated);
					#endif
					for (j = i; j < n - terminated - 1; j++) processes[j] = processes[j + 1];
					--i;
					++terminated;
					#ifdef DEBUG_MODE
						printf("\ntime %dms: processes after remove: %d\n",t,n - terminated);
						debugPrintProcesses(&processes,n - terminated);
					#endif
					continue;
				}
				printf("time %dms: Placed process %c:\n", t, processes[i].proc_id);
				fflush(stdout);
				processes[i].x = pos.x;
				processes[i].y = pos.y;
				processes[i].state = RUNNING;
				processes[i].update_time = t + processes[i].run_time;
				placeProcess_Non(&(mem->board), pos, processes[i].proc_id, processes[i].frames);
				pageTablePrint(&(mem->board), n);
			}
		}
		if (terminated == n) break; // ends simulation when all processes have been terminated
		t++;
	}
	printf("time %dms: Simulator ended (Non-contiguous)", t);
}

int main(int argc, char const *argv[]) {
	if (argc < 2) { // error handling for command-line arguments
		fprintf(stderr, "ERROR: Invalid arugment(s)\nUSAGE: ./a.out <input-file>\n");
		exit(EXIT_FAILURE);
	}
	int n = 0; // the number of processes to simulate
	int t_memmove = 1; // time required to move one frame of memory (in milliseconds)
	process *processes = fileParser(&n, &argv[1]);
	contiguous(&processes, n, t_memmove);
	noncontiguous(&processes, n);
	free(processes);
	return EXIT_SUCCESS;
}