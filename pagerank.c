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
double pr[MAX] = {0};
int numNodes = 0;

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


void pagerank(node *adjList[], int numNodes, int numIterations) {
    
    
    // Run PageRank algorithm for numIterations iterations
    for (int iter = 0; iter < numIterations; iter++) {
        for (int i = 0; i < MAX; i++) {
            if (!nodeExists(adjList, i)) 
                continue;
            if (adjList[i]->numOfNeighbors != 0)
                sum[i] =  adjList[i]->value/adjList[i]->numOfNeighbors;
            else 
                sum[i] = 0;
            //printf("sum of neighbors for %d: %f and numOfNeighbors: %d and value: %f \n",i , sum[i], adjList[i]->numOfNeighbors, adjList[i]->value);    
        } 
        
        for (int i = 0; i < MAX; i++) {
            if (!nodeExists(adjList, i)) 
                continue;
            
            //printf("sum of neighbors for %d: %f\n",i , sum[i]);
            pr[i] = 0;
            node *curr = adjList[i]->next; 
            while (curr != NULL) { // for each neighbor
                pr[i] += sum[curr->vertex];
                //printf("added %f to pr[%d] from vertex %d\n", sum[curr->vertex], i, curr->vertex);
                curr = curr->next;
            }
            //printf("pr for %d: %f\n", i, pr[i]);
            double pagerank = (1.0 - DAMPING_FACTOR) + DAMPING_FACTOR * pr[i];
            adjList[i]->value = pagerank;
            
        }
    }
}

int main(int argc, char **argv) {
        char line[256];
        if (argc < 2) {
            printf("Not enough arguements were given\n");
            return 1;
        }
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
        return 0;
}