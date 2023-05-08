#include <stdlib.h>
#include "IsraeliQueue.h"
#include "stdio.h"
#include "malloc.h"

typedef struct node_t * Node;
struct node_t {
    void* item;
    Node next;
    int friends;
    int rivals;
};

struct IsraeliQueue_t {
    int rivalryThreshold;
    int friendshipThreshold;
    FriendshipFunction *friendshipFunctions;
    ComparisonFunction comparison;
    Node head;
    int queueSize;
};

Node newNode(void* item) {
    Node node = malloc(sizeof(struct node_t));
    if (node != NULL) {
        node->item = item;
        node->next = NULL;
    }
    return node;
}

IsraeliQueue IsraeliQueueCreate(FriendshipFunction *friendshipFunctions, ComparisonFunction comparison, int friendshipThreshold, int rivalryThreshold) {
    IsraeliQueue queue = malloc(sizeof(struct IsraeliQueue_t));
    if (queue != NULL) {
        queue->friendshipThreshold = friendshipThreshold;
        queue->rivalryThreshold = rivalryThreshold;
        queue->comparison = comparison;
        queue->queueSize = 0;
        queue->head = NULL;
        queue->friendshipFunctions = calloc(FRIEND_QUOTA + 1, sizeof(FriendshipFunction));
        if (queue->friendshipFunctions != NULL) {
            int i = 0;
            while (friendshipFunctions[i] != NULL && i < FRIEND_QUOTA) {
                queue->friendshipFunctions[i] = friendshipFunctions[i];
                i++;
            }
            queue->friendshipFunctions[i] = NULL;
        }
        else {
            free(queue);
            queue = NULL;
        }
    }
    return queue;
}

IsraeliQueue IsraeliQueueClone(IsraeliQueue q){
    IsraeliQueue newQueue = malloc(sizeof (IsraeliQueue));
    if(newQueue == NULL){
        //allocating memory failed
        return NULL;
    }
    newQueue->friendshipThreshold = q->friendshipThreshold;
    newQueue->rivalryThreshold = q->rivalryThreshold;
    newQueue->comparison = q->comparison;
    newQueue->friendshipFunctions = q->friendshipFunctions;
    newQueue->head = malloc(sizeof (Node));
    newQueue->head = NULL;

    Node current = q->head;
    Node previous = NULL;
    while(current != NULL){
        Node newNode = malloc(sizeof (Node));
        if(newNode == NULL){
            //allocating memory failed
            return NULL;
        }
        newNode->item=current->item;
        newNode->next=current->next;

        //adding the new node to the queue
        if(previous == NULL){
            newQueue->head= newNode; // which means this is the first node in our queue
        }
        else{
            previous->next=newNode;
        }
        previous = newNode;
        current = current->next;
    }
    newQueue->queueSize = q->queueSize;
    return newQueue;
}

void IsraeliQueueDestroy(IsraeliQueue q) {
    if (q != NULL) {
        Node current = q->head;
        while (current != NULL) {
            Node next = current->next;
            free(current);
            current = next;
        }
        free(q->friendshipFunctions);
        free(q);
    }
}

bool enqueueAux(IsraeliQueue queue, Node head, Node newNode);

IsraeliQueueError IsraeliQueueEnqueue(IsraeliQueue queue, void *item){
    ////////////////////////////////////////////////////////////////
    printf("Enqueue item %d\n", *(int*)item);
    printf("print Q\n");
    Node temp = queue->head;
    while(temp != NULL){
        printf(" %d", *(int*)temp->item);
        temp = temp->next;
    }
    printf("\n");
    ////////////////////////////////////////////////////////////////

    Node newNode = malloc(sizeof (Node));

    if(newNode == NULL){
        return ISRAELIQUEUE_ALLOC_FAILED;
    }
    newNode->item = item;
    newNode->next = NULL;
    newNode->friends = 0;
    newNode->rivals = 0;

    if(queue->head == NULL){
        //this means that the queue is empty and this is the first node we added
        queue->head = newNode;
        queue->queueSize++;
        return ISRAELIQUEUE_SUCCESS;
    }

    if(queue->friendshipFunctions[0] == NULL){
        //no friendship functions
        Node current = queue->head;
        while(current->next != NULL){
            current = current->next;
        }
        current->next = newNode;
        queue->queueSize++;
        return ISRAELIQUEUE_SUCCESS;
    }

    bool enqueueRes = enqueueAux(queue, queue->head, newNode);
    if(!enqueueRes){
        //item was not added
        Node current = queue->head;
        while(current->next != NULL){
            current = current->next;
        }
        current->next = newNode;
        queue->queueSize++;
        return ISRAELIQUEUE_SUCCESS;
    }
    /*else { //the queue is not empty
        Node currentNode = queue->head;
        while(currentNode->next != NULL){
            //to check if there's a friend
            int index=0, friendshipMeasure=0;
            Node friend;
            while(queue->friendshipFunctions[index] != NULL){
                if(queue->friendshipFunctions[index](currentNode, newNode) > queue->friendshipThreshold){ //this means we found a friend
                    friend = currentNode;
                    currentNode = friend->next;

                }

            }
        }
    }
*/
    queue->queueSize++;
    return ISRAELIQUEUE_SUCCESS;
}

bool enqueueAux(IsraeliQueue queue, Node head, Node newNode){
    if(head == NULL){
        head->next = newNode;
        return false;
    }
    Node current = head;
    while(current != NULL){
        printf("current data is %d\n", *(int*)current->item);
        int index = 0, fMeasure = 0;
        Node friend, enemy;
        while(queue->friendshipFunctions[index] != NULL) {
            fMeasure = queue->friendshipFunctions[index](current->item, newNode->item);
            printf("friendship measure = %d current friends = %d\n", fMeasure, current->friends);
            if (fMeasure > queue->friendshipThreshold && current->friends < FRIEND_QUOTA) {
                //this means we found a friend
                printf("Friend found!\n");
                if(current->next == NULL){
                    printf("friend is last\n");
                    //the friend is the last one
                    current->next = newNode;
                    current->friends++;
                    printf("Enqueued after %d, friends of curr = %d\n", *(int*)current->item, current->friends);
                    return true;
                }
                while (current->next != NULL) {
                    int enemyIdx = 0, avg = 0;
                    bool flag = false;
                    while (queue->friendshipFunctions[enemyIdx] != NULL) {
                        int friendshipMeasure = queue->friendshipFunctions[enemyIdx](current->item, newNode->item);
                        if (friendshipMeasure > queue->friendshipThreshold) {
                            //this means we found a friend
                            flag = true;
                            break;
                        } else {
                            avg += friendshipMeasure;
                        }
                        enemyIdx++;
                    }
                    if (!flag) {
                        avg /= enemyIdx;
                    }
                    if (avg < queue->rivalryThreshold) {
                        // found enemy
                        enemy = current;
                        enqueueAux(queue, enemy->next, newNode);
                    }
                    // if no enemies:
                    newNode->next = current->next;
                    current->next = newNode;
                    current->friends++;
                    printf("Enqueued after %d, friends of curr = %d \n", *(int*)current->item, current->friends);
                    printf("rivals of current: %d\n", current->rivals);
                    return true;
                }
            }
            index++;
        }
        current = current->next;
    }

}

IsraeliQueueError IsraeliQueueAddFriendshipMeasure(IsraeliQueue queue, FriendshipFunction newFunction){
    if(queue == NULL){
        return ISRAELI_QUEUE_ERROR;
    }
    if(newFunction == NULL){
        return ISRAELIQUEUE_BAD_PARAM;
    }
    int size = 0;
    while(queue->friendshipFunctions[size] != NULL){
        size++;
    }
    size+=1; // for the new function
    FriendshipFunction* newFriendshipFunctions = realloc(queue->friendshipFunctions,size * sizeof(FriendshipFunction));
    if(newFriendshipFunctions == NULL){
        return ISRAELIQUEUE_ALLOC_FAILED;
    }
    queue->friendshipFunctions = newFriendshipFunctions;
    queue->friendshipFunctions[size - 1] = newFunction;
    queue->friendshipFunctions[size] = NULL;

    return ISRAELIQUEUE_SUCCESS;
}

IsraeliQueue IsraeliQueueMerge(IsraeliQueue* qarr, ComparisonFunction compare_function) {
    if (qarr == NULL || compare_function == NULL) {
        return NULL;
    }

    int num_queues = 0;
    for (int i = 0; qarr[i] != NULL; i++) {
        num_queues++;
    }

    IsraeliQueue merged = IsraeliQueueCreate(NULL, compare_function, 0, 0);
    if (merged == NULL) {
        return NULL;
    }

    int current_queue = 0;
    while (current_queue < num_queues) {
        IsraeliQueue current = qarr[current_queue];
        if (current != NULL && IsraeliQueueSize(current) > 0) {
            void* item = IsraeliQueueDequeue(current);
            if (item != NULL) {
                IsraeliQueueEnqueue(merged, item);
            } else {
                // Dequeue failed, destroy merged queue and return NULL
                IsraeliQueueDestroy(merged);
                return NULL;
            }
        } else {
            current_queue++;
        }
    }

    // If any queues still have items, enqueue them in the order of qarr
    for (int i = current_queue; i < num_queues; i++) {
        IsraeliQueue current = qarr[i];
        while (current != NULL && IsraeliQueueSize(current) > 0) {
            void* item = IsraeliQueueDequeue(current);
            if (item != NULL) {
                IsraeliQueueEnqueue(merged, item);
            } else {
                // Dequeue failed, destroy merged queue and return NULL
                IsraeliQueueDestroy(merged);
                return NULL;
            }
        }
    }

    return merged;
}

IsraeliQueueError IsraeliQueueImprovePositions(IsraeliQueue q) {
    if (q == NULL) {
        return ISRAELIQUEUE_BAD_PARAM;
    }

    int size = IsraeliQueueSize(q);
    if (size < 2) {
        return ISRAELIQUEUE_SUCCESS;
    }

    void** items = malloc(size * sizeof(void*));
    if (items == NULL) {
        return ISRAELIQUEUE_ALLOC_FAILED;
    }

    // Dequeue all items and store them in an array
    for (int i = 0; i < size; i++) {
        void* item = IsraeliQueueDequeue(q);
        if (item == NULL) {
            // Dequeue failed, destroy array and return error
            for (int j = 0; j < i; j++) {
                free(items[j]);
            }
            free(items);
            return ISRAELIQUEUE_BAD_PARAM;
        }
        items[i] = item;
    }

    // Re-enqueue items in the correct order
    for (int i = 0; i < size; i++) {
        IsraeliQueueEnqueue(q, items[i]);
    }

    free(items);
    return ISRAELIQUEUE_SUCCESS;
}

bool IsraeliQueueContains(IsraeliQueue queue, void *item){
    if(queue == NULL || item == NULL){
        return false;
    }
    Node currentNode = queue->head;
    while(currentNode->item != NULL){
        if(currentNode->item == item){
            return true;
        }
        currentNode = currentNode->next;
    }
    return false;
}
int IsraeliQueueSize(IsraeliQueue queue) {
    if (queue == NULL) {
        return 0;
    }
    return queue->queueSize;
}

IsraeliQueueError IsraeliQueueUpdateRivalryThreshold(IsraeliQueue queue, int newThreshold) {
    if (queue == NULL || newThreshold < 0) {
        return ISRAELIQUEUE_BAD_PARAM;
    }
    queue->rivalryThreshold = newThreshold;
    return ISRAELIQUEUE_SUCCESS;
}

IsraeliQueueError IsraeliQueueUpdateFriendshipThreshold(IsraeliQueue queue, int newThreshold) {
    if (queue == NULL || newThreshold < 0) {
        return ISRAELIQUEUE_BAD_PARAM;
    }
    queue->friendshipThreshold = newThreshold;
    return ISRAELIQUEUE_SUCCESS;
}

void* IsraeliQueueDequeue(IsraeliQueue queue){ //TODO
    if(queue == NULL || queue->head == NULL){
        return NULL;
    }
    Node oldHead = queue->head;
    if(queue->queueSize == 1){
        queue->head = NULL;
    }
    else{
        queue->head = queue->head->next;
    }
    void* data = oldHead->item;
    free(oldHead);
    queue->queueSize--;
    return data;
}


