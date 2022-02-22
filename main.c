/* 
COP 3502C Assignment 2 
This program is written by: Jennifer Nguyen
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leak_detector_c.h"

// structure for soldier queues and nodes
struct soldierNode
{
    int prisNum;
    struct soldierNode* next;
    struct soldierNode* prev;
};

struct soldierQueue
{
    struct soldierNode* front;
    struct soldierNode* back;
};

// structure for ground queue and its nodes
struct groundNode
{
    int order;
    char *name;
    int nodeCount;
    struct soldierQueue* prisoner;
    int k;
    int th;
    struct groundNode* next;
    struct groundNode* prev;
};

typedef struct queue
{
    struct groundNode* front;
    struct groundNode* back;
}queue;

// set structures to NULL
void init(struct queue* arena)
{
    arena->front = NULL;
    arena->back = NULL;
}

void initPris(struct soldierQueue* prisoner)
{
    prisoner->front = NULL;
    prisoner->back = NULL;
}

// checks if queue is empty
int isEmpty(struct queue* qPtr)
{
    if (qPtr->front == NULL)
        return 1;
    return 0;
}

void release_memory(queue** q, struct soldierQueue* prisoner, int numGroup)
{
    int i;
    for(i = 0; i < numGroup; i++)
    {
        free(q[i]->front->name);
        free(q[i]->front);
        free(q[i]);
    }
    
    free(q);
    free(prisoner);
    return;
}

// enqueues solder node
struct soldierQueue* enqueue(struct soldierNode* solTemp, struct soldierQueue* prisoner)
{
    // if front is null insert front
    if (prisoner->front == NULL && prisoner->front == prisoner->back)
    {
        prisoner->front = prisoner->back = solTemp;
        prisoner->front->next = prisoner->back->next = NULL;
        prisoner->front->prev = prisoner->back->prev = NULL;
    }
    else
    {
        prisoner->back->next = solTemp;
        solTemp->prev = prisoner->back;
        prisoner->back = solTemp;
        prisoner->front->prev = prisoner->back;
        prisoner->back->next = prisoner->front;
    }
    
    return prisoner;
}

// for pahse 2 delete soldierNode
void deleteSol(queue *q, struct soldierNode* max)
{
    // if arguments are null do nothing
    if (q->front->prisoner->front == NULL || max == NULL)
    {
        return;
    }
    // if max is at the head
    if (q->front->prisoner->front == max)
    {
        q->front->prisoner->front = max->next;
    }
    // if max is not last
    if (max->next != NULL)
    {
        max->next->prev = max->prev;
    }
    // if max is not head
    if (max->prev != NULL)
    {
        max->prev->next = max->next;
    }
    
    printf("Executed Soldier %d from line %d\n", max->prisNum, q->front->order);
    
    free(max);
}

// creates one solder node
struct soldierNode* createSoldier(int sequence)
{
    struct soldierNode* solTemp;
    solTemp = (struct soldierNode*) malloc(sizeof(struct soldierNode));
    
    if (solTemp != NULL)
    {
        solTemp->prisNum = sequence;
        solTemp->next = NULL;
    }
    return solTemp;
}

// create solder queue backwards
void createReverseCircle(queue *q)
{
    int i;
    
    q->front->prisoner =  malloc(10 * sizeof(struct soldierQueue));
    for (i = q->front->nodeCount; i > 0; i--)
        {
            q->front->prisoner = enqueue(createSoldier(i), q->front->prisoner);
        }
}


void rearrangeCircle(queue* q)
{
    struct soldierNode* A = q->front->prisoner->front;
    struct soldierNode* temp = NULL;
    int i;
    if (q->front->prisoner != NULL)
    {
        // use temp to suspend a node then switch until list is reversed
        for (i = 0; i < q->front->nodeCount; i++)
        {
            temp = A->prev;
            A->prev = A->next;
            A->next = temp;
            A = A->prev;
        }
        // change the head
        temp = q->front->prisoner->front;
        q->front->prisoner->front =  q->front->prisoner->back;
        q->front->prisoner->back = temp;
    }
}

void phase1(queue *q, FILE *ofp)
{
    int count = 1, i, post1;
    struct soldierNode* head = q->front->prisoner->front;
    struct soldierNode* current = head;
    struct soldierNode* iter = current;
    if (q != NULL && q->front->prisoner != NULL)
    {
        printf("Line# %d %s\n", q->front->order, q->front->name);
        fprintf(ofp, "Line# %d %s\n", q->front->order, q->front->name);
        while (q->front->nodeCount > q->front->th)
        {
            // if correct position removed node
            if (count % q->front->k == 0 )
            {
                
                // deletes head
                if (current == head)
                {
                    iter = head;
                    while (iter->next != head)
                        iter = iter->next;
                    head = current->next;
                    iter->next = head;
                    printf("Soldier# %d executed\n", current->prisNum);
                    fprintf(ofp, "Soldier# %d executed\n", current->prisNum);
                    free(current);
                    current = current->next;
                }
                // delete last
                else if (current->next == head)
                {
                    iter->next = head;
                    printf("Soldier# %d executed\n", current->prisNum);
                    fprintf(ofp, "Soldier# %d executed\n", current->prisNum);
                    free(current);
                    current = current->next;
                }
                // middle nodes
                else
                {
                    iter->next = current->next;
                    printf("Soldier# %d executed\n", current->prisNum);
                    fprintf(ofp, "Soldier# %d executed\n", current->prisNum);
                   
                    //dead = current;
                    free(current);
                    current = current->next;
                }
                // update nodeCount
                q->front->nodeCount--;
            }
            // if incorrect postion, move on
            else if (count % q->front->k != 0)
            {
                iter = current;
                current = current->next;
            }
            count++;
        }
        // transverse through prisoners
        q->front->prisoner->front = q->front->prisoner->back->next;
    }
    printf("\n");
}

void phase2(queue** q, FILE *ofp, int postOne, int numGroup)
{
    int i, j, groundOr, groundPlace;
    
   
    while (postOne > 1)
    {
        struct soldierNode* max = NULL;
        for (i = 0; i < numGroup; i++)
        {
            
            // move empty things along
            if (isEmpty(q[i]) || q[i]->front->nodeCount == 0 || q[i]->front->prisoner == NULL)
            {
                continue;
            }
            else if (max == NULL && q[i]->front->prisoner->front != NULL)
            {
                max = q[i]->front->prisoner->front;
                groundOr = q[i]->front->order;
                groundPlace = i;
            }
           

            // search for max
            if (max->prisNum < q[i]->front->prisoner->front->prisNum)
                {
                    max = q[i]->front->prisoner->front;
                    groundOr = q[i]->front->order;
                    groundPlace = i;
                }
              
            // if max is same as another groups, change max to the smaller group number
            if (max->prisNum == q[i]->front->prisoner->front->prisNum && q[i]->front->order < groundOr)
                {
                    max = q[i]->front->prisoner->front;
                    groundOr = q[i]->front->order;
                    groundPlace = i;
                }
        }
        deleteSol(q[groundPlace], max);
        q[groundPlace]->front->nodeCount--;
        if (q[groundPlace]->front->nodeCount == 0)
        {
            free(q[groundPlace]->front->prisoner);
            q[groundPlace]->front->prisoner = NULL;
        }
        postOne--;
    }
    // transverse the groups to find winner
    for (i = 0; i < numGroup; i++)
       {
           if (q[i]->front->nodeCount != 0)
           {
               if (q[i]->front->prisoner->front != NULL)
               {
                   printf("Soldier %d from line %d will survive", q[i]->front->prisoner->front->prisNum, q[i]->front->order);
                   fprintf(ofp, "Soldier %d from line %d will survive", q[i]->front->prisoner->front->prisNum, q[i]->front->order);
                   free(q[i]->front->prisoner->front);
                   free(q[i]->front->prisoner);
               }
           }
           else
            continue;
       }
       
}
     

// creates larger structure of ground
queue* createGround(struct queue* qPtr, int grouOrder, char *grouName, int numSoldier, int interval, int survivor)
{
    struct groundNode* temp;
    int i;
    temp = (struct groundNode*) malloc(sizeof(struct groundNode));
    
    if (temp != NULL)
    {
        temp->order = grouOrder;
        temp->name = malloc(50 * sizeof(char));
        strcpy(temp->name, grouName);
        temp->nodeCount = numSoldier;
        
        temp->k = interval;
        temp->th = survivor;
        temp->next = NULL;
        
        if (qPtr->back != NULL)
            qPtr->back->next = temp;
            
        qPtr->back = temp;
        
        if (qPtr->front == NULL)
            qPtr->front = temp;
    }
    return qPtr;
}


void display(struct queue *q, FILE *ofp)
{
    struct groundNode *current = q->front;
    //struct soldierNode *t = q->front->prisoner;
    int i;
     
    printf("%d ", current->order);
    fprintf(ofp, "%d ", current->order);
    printf("%s", current->name);
    fprintf(ofp, "%s ", current->name);
    for(i = 0; i < current->nodeCount; i++)
    {
        printf(" ");
        printf("%d", current->prisoner->front->prisNum);
        fprintf(ofp, "%d", current->prisoner->front->prisNum);
        current->prisoner->front = current->prisoner->front->next;
    }
    printf("\n");
    fprintf(ofp, "\n");
}

int main()
{
    atexit(report_mem_leak);
    
    FILE *ifp, *ofp;
    ifp = fopen("in.txt", "r");
    ofp = fopen("out.txt", "w");
    
    // struct queue* 
    queue **arena = (struct queue**) malloc(10 * sizeof(struct queue*));
    //struct soldierQueue* prisoner;
    struct soldierQueue* prisoner = (struct soldierQueue*) malloc(10 * sizeof(struct soldierQueue));
    initPris(prisoner);
    
    int i, j, z, numGroup, order, nodeCount, k, th, postOne;
    char name[50];
    
    fscanf(ifp, "%d", &numGroup);
    
    for (i = 0; i < numGroup; i++)
    {
        arena[i] = (struct queue*) malloc(sizeof(struct queue));
    
        init(arena[i]);
        
        fscanf(ifp, "%d", &order);
        fscanf(ifp, "%s", name);
        fscanf(ifp, "%d", &nodeCount);
        fscanf(ifp, "%d", &k);
        fscanf(ifp, "%d", &th);
        arena[i] = createGround(arena[i], order, name, nodeCount, k , th);
        createReverseCircle(arena[i]);
    }
    printf("Initial nonempty lists status\n");
    fprintf(ofp, "Initial nonempty lists status\n");
    for (j = 1; j < 11; j++)
    {
        for (z = 0; z < numGroup; z++)
        {
            if (arena[z]->front->order == j)
                display(arena[z], ofp);
                
        }
    }
    printf("\n");
    for (i = 0; i < numGroup; i++)
        rearrangeCircle(arena[i]);
    fprintf(ofp, "\n");
    
    // display in order
    printf("After ordering nonempty lists status\n");
    fprintf(ofp, "After ordering nonempty lists status\n");
    for (j = 1; j < 11; j++)
    {
        for (z = 0; z < numGroup; z++)
        {
            if (arena[z]->front->order == j)
                display(arena[z], ofp);
        }
    }
    fprintf(ofp, "\n");
    printf("Phase1 execution \n");
   // Phase 1
    for (j = 1; j < 11; j++)
    {
        for (z = 0; z < numGroup; z++)
        {
            if (arena[z]->front->order == j)
            {
                phase1(arena[z], ofp);
                fprintf(ofp, "\n");
                postOne += arena[z]->front->nodeCount;
            }
        }
    }
    for (i = 0; i < numGroup; i++)
    {
        arena[i]->front->nodeCount = arena[i]->front->th;
    }
    
    fprintf(ofp, "\n");
    // phase 2
    printf("Phase2 execution\n");
    phase2(arena, ofp, postOne, numGroup);
    fprintf(ofp, "\n");
    
    release_memory(arena, prisoner, numGroup);
    
    return 0;
}
