

#include <stdbool.h>
#include <stdint.h>
#define QUEUE_DEPTH 10

typedef struct {int16_t head; int16_t load; int16_t buffer[QUEUE_DEPTH+3];} Queue;


void init_queue(Queue* this);

bool enqueue(Queue* this, int16_t data);

bool dequeue(Queue* this, int16_t* data);

void monitor_queues();

void init_all_queues();


