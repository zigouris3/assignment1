#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
// #include <unistd.h>
#define MAX 50000  // max number of threads
#define DAMPING_FACTOR 0.85
#define MAX_THREADS 4


// the node struct, in which we have the vertex and the next node is a list of neighbors
typedef struct node{
  long numOfNeighbors;
  long vertex;
  double value;
  struct node* next;
}node;


node *adjList[MAX];
double sum[MAX];
double pr[MAX];
pthread_barrier_t barrier;

typedef struct thread_params {
    int start;
    int end;
} thread_params;

thread_params params[MAX_THREADS];

int nodeExists(int vertex) {
    if (adjList[vertex] == NULL)
        return 0;
    return 1;
}

node *createNode(long vertex) {
    node *newNode = malloc(sizeof(node));
    newNode->vertex = vertex;
    newNode->value = 1.0;
    newNode->next = NULL;
    newNode->numOfNeighbors = 0;
    return newNode;
}


void addEdge(int src, int dest) {
    // Create a new node with the destination vertex
    node* newNode = createNode(src);
    // Traverse to the end of the adjacency list at the source vertex
    
    node* curr = adjList[dest];
    while (curr->next != NULL) {
        curr = curr->next;
    }
    // Add the new node to the end of the linked list
    curr->next = newNode;
    // Increment the numOfNeighbors variable of the destination node
    adjList[src]->numOfNeighbors++; 
}


void* pagerank(void* arg) {
    // Cast the void* argument to the correct type
    thread_params* params = (thread_params*) arg;
    
    //printf("Starting value IN PAGERANK: %d", params->start);
    
    for (int i = params->start; i < params->end; i++) {
        if (!nodeExists(i)) 
            continue;
        if (adjList[i]->numOfNeighbors != 0){
                //printf("Node %d value: %f\n", i, adjList[i]->value);
                sum[i] =  adjList[i]->value/adjList[i]->numOfNeighbors;
                //printf("sum value: %f\n", i, sum[i]);
        }
            else 
                sum[i] = 0;
    }
    pthread_barrier_wait(&barrier);
    // Reset the pagerank value to 0 for this node
    for (int i = params->start; i < params->end; i++) {
        if (!nodeExists(i)) 
            continue;
        pr[i]=0;    
        // Calculate the pagerank value for this node based on its neighbors' values
        node *curr = adjList[i]->next; 
        while (curr != NULL) { // for each neighbor 
            pr[i] += sum[curr->vertex];
            curr = curr->next;
        }
        double pagerankval = (1.0 - DAMPING_FACTOR) + DAMPING_FACTOR * pr[i];
        adjList[i]->value = pagerankval;
    }
}


int main(int argc, char **argv) {
        int numNodes = 0;
        int numThreads = 0;
        int chunkSize = 0;
        pthread_t threads[MAX_THREADS];
        char line[256];
        if (argc < 3) {
            printf("Not enough arguements were given\n");
            return 1;
        }
        numThreads = atoi(argv[2]);
        FILE *fp = fopen(argv[1], "r"); // open file
        if (fp == NULL) {
            printf("File not found\n");
            return 1;
        }
        
        // Initialize adjacency list
         while (fgets(line, sizeof(line), fp)) {
            if (line[0] == '#') {
                continue;  // skip this line and move on to the next one
            }
            int src, dest;
            sscanf(line, "%d %d", &src, &dest);
            if (adjList[src] == NULL) {
                adjList[src] = createNode(src);
                numNodes++;
            }
            if (adjList[dest] == NULL) {
                adjList[dest] = createNode(dest);
                numNodes++;
            }
        }   


        
        // Add edges to graph
        rewind(fp);
        while (fgets(line, sizeof(line), fp)) {
            if (line[0] == '#') {
                continue;  // skip this line and move on to the next one
            }
            int src, dest;
            sscanf(line, "%d %d", &src, &dest);
            addEdge(src, dest); // add edge to graph
        }   
        
        clock_t start = clock();
        if (numThreads < 1 || numThreads > 4    ) {
            printf("Invalid number of threads. Using 4 threads instead.\n");
            numThreads = 4;
        }
        chunkSize = numNodes / numThreads;
        pthread_barrier_init(&barrier, NULL, numThreads);
    
        for (int iter = 0; iter < 500; iter++) {
        // Create and start the threads
            for (int i = 0; i < numThreads; i++) {
                params[i].start = i * chunkSize;
                params[i].end = (i == numThreads - 1) ? numNodes : params[i].start + chunkSize; // last thread gets the remainder
                pthread_create(&threads[i], NULL, &pagerank, (void*) &params[i]);
            }
        
            // Wait for the threads to finish
            for (int i = 0; i < numThreads; i++) {
                pthread_join(threads[i], NULL);
            }
        }
        //pthread_exit(NULL);
        // Destroy the barrier
        pthread_barrier_destroy(&barrier);
        clock_t end = clock();
        printf("Time: %f seconds \n", (double)(end - start)/CLOCKS_PER_SEC);
        //pagerank(adjList, numNodes, 500);
        // Write pagerank scores to file
        FILE *fop = fopen("output.csv", "w");
        fprintf(fop, "node,pagerank\n");
        for (int i = 0; i < MAX; i++) {
            if(adjList[i] != NULL){
                fprintf(fop, "%d,%.2f\n", i, adjList[i]->value);
            }
        }
        fclose(fop);
        
        return 0;
}