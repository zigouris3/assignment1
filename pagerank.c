#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
// #include <unistd.h>
#define MAX 50000  // max number of threads
#define DAMPING_FACTOR 0.85
#define print(fun,msg); printf("test in %s, %s\n", fun, msg);


// the node struct, in which we have the vertex and the next node is a list of neighbors
typedef struct node{
  long numOfNeighbors;
  long vertex;
  double value;
  // pthread_mutex_t lock;
  struct node* next;
}node;


node *adjList[MAX];
double sum[MAX] = {0};
//double pr[MAX] = {0};
int numNodes = 0;
int numThreads = 0;
int chunkSize = 0;
pthread_barrier_t barrier;

typedef struct thread_params {
    int start;
    int end;
    node** adjList;
} thread_params;



int nodeExists(node *adjList[], int vertex) {
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
    //printf("vertex created: %d\n", newNode->vertex);
    //pthread_mutex_init(&newNode->lock, NULL);
    return newNode;
}


void addEdge(node** adjList, int src, int dest) {
    // Create a new node with the destination vertex
    node* newNode = createNode(src);
    //print("addEdge", 2);
    // Traverse to the end of the adjacency list at the source vertex
    
    node* curr = adjList[dest];
    //print("addEdge", 3);
    //printf("source vertex: %d\nNow traversing neighbor list:\n", curr->vertex);
    while (curr->next != NULL) {
        curr = curr->next;
        //printf("%d\n", curr->vertex);
    }
    //print("addEdge", 4);
    // Add the new node to the end of the linked list
    curr->next = newNode;
    //printf("newNode is added to the end of the linked list: %d\n", newNode->vertex);
    //print("addEdge", 5);
    // Increment the numOfNeighbors variable of the destination node
    adjList[src]->numOfNeighbors++;
    // print("addEdge", 6);    
}


void* pagerank_thread(void* arg) {
    // Cast the void* argument to the correct type
    thread_params* params = (thread_params*) arg;
    
    // Iterate through the nodes within the range
    for (int i = params->start; i < params->end; i++) {
        if (!nodeExists(params->adjList, i)) 
            continue;
        
        // Reset the pagerank value to 0 for this node
        double pr = 0;
        
        // Calculate the pagerank value for this node based on its neighbors' values
        node *curr = params->adjList[i]->next; 
        while (curr != NULL) { // for each neighbor
            if (params->adjList[i]->numOfNeighbors != 0) {
                pr += params->adjList[curr->vertex]->value / params->adjList[i]->numOfNeighbors;
            }
            curr = curr->next;
        }
        double pagerank = (1.0 - DAMPING_FACTOR) + DAMPING_FACTOR * pr;
        params->adjList[i]->value = pagerank;
    }
    
    // Exit the thread
    pthread_exit(NULL);
}

void pagerank(node *adjList[], int numNodes, int numIterations) {
    // Create an array of thread IDs and thread parameters
    pthread_t threads[numThreads];
    thread_params params[numThreads];
    
    // Initialize the barrier
    pthread_barrier_init(&barrier, NULL, numThreads);

    // Run PageRank algorithm for numIterations iterations
    for (int iter = 0; iter < numIterations; iter++) {
        // Create and start the threads
        for (int i = 0; i < numThreads; i++) {
            params[i].start = i * chunkSize;
            params[i].end = (i == numThreads - 1) ? numNodes : params[i].start + chunkSize; // last thread gets the remainder as to not get out of bounds
            params[i].adjList = adjList;
            pthread_create(&threads[i], NULL, pagerank_thread, (void*) &params[i]);
        }
        
        // Wait for the threads to finish
        for (int i = 0; i < numThreads; i++) {
            pthread_join(threads[i], NULL);
        }
    }
    
    // Destroy the barrier
    pthread_barrier_destroy(&barrier);
}



int main(int argc, char **argv) {
        clock_t start = clock();
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
        //printGraph(adjList);
        for (int i = 0; i < MAX; i++) {
            if (adjList[i] != NULL) {
                //printf("vertex : %d\n", adjList[i]->vertex);
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
            addEdge(adjList, src, dest); // add edge to graph
        }   
        printf("after main while loop\n");
        //printGraph(adjList);
        printf("%d nodes\n", numNodes);
        if (numThreads > 4 || numThreads < 1) {
            numThreads = 4;
        }
        chunkSize = numNodes / numThreads;
        pthread_barrier_init(&barrier, NULL, numThreads);
        // Run PageRank algorithm
        pagerank(adjList, numNodes, 500);
        // Write pagerank scores to file
        FILE *fop = fopen("output.csv", "w");
        fprintf(fop, "node,pagerank\n");
        for (int i = 0; i < MAX; i++) {
            if(adjList[i] != NULL){
                fprintf(fop, "%d,%.2f\n", i, adjList[i]->value);
            }
        }
        fclose(fop);
        clock_t end = clock();
        printf("Time: %f seconds \n", (double)(end - start)/CLOCKS_PER_SEC);
        return 0;
}