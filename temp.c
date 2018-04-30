//
// Created by Ruowen Qin on 2018/4/29.
//

position findPlace(char ***board) { // find the first empty places
    position pos = {.x = -1, .y =-1};
    for (int i = 0; i < LINES; ++i) {
        for (int j = 0; j < FRAMES_PER_LINE; ++j) {
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

void placeProcess_Non(char ***board, position pos, char id, int frames) {// places process in memory board
    for (int i = pos.y; i < LINES; ++i) {
        for (int j = 0; j < FRAMES_PER_LINE; ++j) {
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
        for (i = low + 1; i < high; ++i) {
            if ((*queue)[i] < (*queue)[low]) {
                swap(queue, ++last, i);
            }
        }
        swap(queue, last, low);
        queueSort(queue, low, last - 1);
        queueSort(queue, last + 1, high);
    }
}

int partition(char **queue, int low, int high) {
    char key;
    key = (*queue)[low];
    while (low < high) {
        while (low < high && (*queue)[high] >= key) {
            high--;
        }
        if (low < high) {
            (*queue)[low++] = (*queue)[high];
        }
        while (low < high && (*queue)[low] <= key) {
            low++;
        }
        if (low < high) {
            (*queue)[high--] = (*queue)[low];
        }
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
    for (int i = 0; i < length; ++i) {
        if (list[i] == item) {
            return 1;
        }
    }
    return 0;
}

void pageTablePrint(char ***board, int n) {
    int count = 0;
    printf("PAGE TABLE [page,frame]:\n");
    char *idList = (char *) (calloc((size_t) n, sizeof(char)));
    for (int i = 0; i < LINES; ++i) {
        for (int j = 0; j < FRAMES_PER_LINE; ++j) {
            if ((*board)[i][j] != '.') {
                if (!findInList(idList, (*board)[i][j], count)) {
                    idList[count] = (*board)[i][j];
                    count++;
                }
            }
        }
    }
    quick_sort(&idList, 0, count - 1);
    for (int k = 0; k < count; ++k) {
        int count_ID = 0;
        int count_10 = 0;
        printf("%c: ", idList[k]);
        for (int i = 0; i < LINES; ++i) {
            for (int j = 0; j < FRAMES_PER_LINE; ++j) {
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
    int count = 0;
    for (int i = 0; i < LINES; ++i) {
        for (int j = 0; j < FRAMES_PER_LINE; ++j) {
            if ((*board)[i][j] == '.') {
                count++;
            }
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
