// Kaylan Johnson <johnsk18>
// Xiao Jiang <jiangx5>
// Ruowen Qin <qinr>

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define FRAMES_PER_LINE 32
#define LINES 8

typedef struct process { // struct for storing the data of a process
	char proc_id; // process id 
	int p_mem; // number of memory frames
	int arr_time; // arrival time
	int run_time; // run time
} process;

typedef struct memory {
	char** mem;
	int frames_per_line;
	int lines;
} memory;

void error() { // outputs error
	perror("ERROR");
	exit(EXIT_FAILURE);
}

void debugPrintProcesses(process** processes, int n) {
	int i;
	for (i = 0; i < n; i++) {
		printf("%c %5d%3c%5d/%-5d\n",(*processes)[i].proc_id,(*processes)[i].p_mem,' ',(*processes)[i].arr_time,(*processes)[i].run_time);
		fflush(stdout);
	}
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
			processes[*n].p_mem = atoi(buffer2);
			do {
				memset(buffer2,'\0',len);
				for (++i; i < len; i++) if (isdigit(buffer[i])) break; // sets i to arr_time starting digit position
				if (i >= len) break;
				if (multiple_times) {
					(*n)++;
					processes[*n].proc_id = processes[*n - 1].proc_id;
					processes[*n].p_mem = processes[*n - 1].p_mem;
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
/*
void best_fit(process** processes, int n, int t_memmove) {
	int i, real_t = 0, change = 0, terminated = 0, context_switching = 0, ready_capacity = 0, wait_capacity = 0;
	process** ready_queue = createQueue(n);
	process** wait_array = createQueue(n);
	process* CPU = NULL;
	printf("time %dms: Simulator started for FCFS ",real_t);
	fflush(stdout);
	printQueue(&ready_queue,ready_capacity);
	while (terminated < n) {
		for (i = 0; i < n - terminated; i++) { // loop for processes leaving the CPU and getting blocked
			if (CPU != NULL && CPU->update_time == real_t && CPU->state == RUNNING) { // process finished with CPU burst
				if (CPU->num_bursts == 0) {
					printf("time %dms: Process %c terminated ",real_t,CPU->proc_id);
					fflush(stdout);
				} else {
					printf("time %dms: Process %c completed a CPU burst; %d burst%s to go ",real_t,CPU->proc_id,CPU->num_bursts, CPU->num_bursts == 1 ? "" : "s");
					fflush(stdout);
					printQueue(&ready_queue,ready_capacity);
					printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms ",real_t,CPU->proc_id,real_t + (t_cs / 2) + CPU->io_time);
					fflush(stdout);
				}
				printQueue(&ready_queue,ready_capacity);
				CPU->update_time += (t_cs / 2);
				CPU->state = CS_REMOVE;
				context_switching = 1;
				change = 1;
			}
			if (CPU != NULL && CPU->update_time == real_t && CPU->state == CS_REMOVE) { // process finished being context switched out of CPU, to be either termianted or blocked
				if (CPU->num_bursts == 0) terminated++;
				else {
					int pos = addProcess(&wait_array,*CPU,n);
					wait_array[pos]->update_time += wait_array[pos]->io_time;
					wait_array[pos]->state = BLOCKED;
					wait_capacity++;
				}
				*sum_turnaround_time += real_t - CPU->turnaround_start_time;
				#ifdef DEBUG_MODE
					printf("\nturnaround time for %c: %dms\n\n", CPU->proc_id, real_t - CPU->turnaround_start_time);
					fflush(stdout);
				#endif
				context_switching = 0;
				free(CPU);
				CPU = NULL;
				change = 1;
			}
			if (wait_array[i] != NULL && wait_array[i]->update_time == real_t && wait_array[i]->state == BLOCKED) { // process is finished with I/O and placed on ready queue
				int pos = addProcess(&ready_queue,*(wait_array[i]),n);
				free(wait_array[i]);
				wait_array[i] = NULL;
				ready_queue[pos]->turnaround_start_time = real_t;
				ready_queue[pos]->wait_start_time = real_t;
				ready_queue[pos]->update_time = real_t;
				ready_queue[pos]->state = READY;
				ready_capacity++;
				wait_capacity--;
				printf("time %dms: Process %c completed I/O; added to ready queue ",real_t,ready_queue[pos]->proc_id);
				fflush(stdout);
				printQueue(&ready_queue,ready_capacity);
				change = 1;
			}
			updateQueue(&ready_queue,n);
			updateQueue(&wait_array,n);
		}
		for (i = 0; i < n - terminated; i++) { // loop for processes arriving and entering the CPU
			if (CPU != NULL && CPU->update_time == real_t && CPU->state == CS_BRING) { // proccess finished context switching into the CPU
				printf("time %dms: Process %c started using the CPU ",real_t,CPU->proc_id);
				fflush(stdout);
				printQueue(&ready_queue,ready_capacity);
				CPU->update_time += CPU->cpu_burst_time;
				CPU->state = RUNNING;
				(CPU->num_bursts)--;
				context_switching = 0;
				change = 1;
			}
			if (ready_queue[i] != NULL && ready_queue[i]->update_time - (t_cs / 2) + 1 == real_t && ready_queue[i]->state == CS_BRING) { // process moves out of the ready queue, context switching into the CPU
				CPU = (process*)calloc(1,sizeof(process));
				memcpy(CPU,ready_queue[i],sizeof(process));
				free(ready_queue[i]);
				ready_queue[i] = NULL;
				ready_capacity--;
				updateQueue(&ready_queue,n);
				change = 1;
			}
			if ((*processes)[i].initial_arrive_time == real_t && (*processes)[i].state == ARRIVING) { // process has arrived 
				int pos = addProcess(&ready_queue,(*processes)[i],n);
				ready_queue[pos]->turnaround_start_time = real_t;
				ready_queue[pos]->wait_start_time = real_t;
				ready_queue[pos]->update_time = real_t;
				ready_queue[pos]->state = READY;
				ready_capacity++;
				printf("time %dms: Process %c arrived and added to ready queue ",real_t,ready_queue[pos]->proc_id);
				fflush(stdout);
				printQueue(&ready_queue,ready_capacity);
				change = 1;
			}
			if (CPU == NULL && context_switching == 0 && ready_queue[i] != NULL && ready_queue[i]->state == READY) { // process is able to use the CPU, beginning to context switch
				*sum_wait_time += real_t - ready_queue[i]->wait_start_time;
				#ifdef DEBUG_MODE
					printf("\nwait time for %c: %dms\n\n", ready_queue[i]->proc_id, real_t - ready_queue[i]->wait_start_time);
					fflush(stdout);
				#endif
				(*context_swtiches)++;
				context_switching = 1;
				ready_queue[i]->state = CS_BRING;
				ready_queue[i]->update_time = real_t + (t_cs / 2);
				change = 1;
			}
			updateQueue(&ready_queue,n);
		}
		if (change) { // if a process has been updated
			change = 0;
			qsort(ready_queue,ready_capacity,sizeof(process*),comparator); // clears ties
			#ifdef DEBUG_MODE
				if (terminated < n) { // debug prints of CPU, ready queue, and wait array
					printf("\n--- Printing at time %dms\n%s CPU\n",real_t, CPU != NULL ? "Printing" : "Empty");
					fflush(stdout);
					if (CPU != NULL) {
						process** temp = &CPU;
						debugPrintQueue(&temp,1);
						temp = NULL;
					}
					printf("%s Ready Queue\n", ready_capacity > 0 ? "Printing" : "Empty");
					fflush(stdout);
					if (ready_capacity > 0)	debugPrintQueue(&ready_queue,ready_capacity);
					printf("%s Wait Array\n", wait_capacity > 0 ? "Printing" : "Empty");
					fflush(stdout);
					if (wait_capacity > 0) debugPrintQueue(&wait_array,wait_capacity);
					printf("--------------------------\n\n");
					fflush(stdout);
				}
			#endif
		}
		if (terminated == n) break; // ends simulation when all processes have been terminated
		real_t++;
	}
	printf("time %dms: Simulator ended for FCFS\n",real_t);
	fflush(stdout);
	if (CPU != NULL) free(CPU);
	freeQueue(&ready_queue,n);
	freeQueue(&wait_array,n);
}
*/
int main(int argc, char const *argv[]) {
	if (argc < 2) { // error handling for command-line arguments
		fprintf(stderr, "ERROR: Invalid arugment(s)\nUSAGE: ./a.out <input-file>\n");
		exit(EXIT_FAILURE);
	}
	int n = 0; // the number of processes to simulate
	// int t_memmove = 1; // time required to move one frame of memory (in milliseconds)
	process* processes = fileParser(&n, &argv[1]);
	free(processes);
	return EXIT_SUCCESS;
}