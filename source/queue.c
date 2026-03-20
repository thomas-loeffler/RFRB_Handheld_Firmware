
////////////////////////////////////////////////////////////////////////////////
//								INCLUDE FILES								  //
////////////////////////////////////////////////////////////////////////////////


#include "queue.h"



////////////////////////////////////////////////////////////////////////////////
//							  QUEUE INSTANCES								  //
////////////////////////////////////////////////////////////////////////////////

Queue Mechanum_q;		
Queue TX_q;	



////////////////////////////////////////////////////////////////////////////////
//									FUNCTIONS								  //
////////////////////////////////////////////////////////////////////////////////


// Initializing the Queue with 0xBAADFOOD at the end of the buffer
void init_queue(Queue* this){
	this->head = 0;
	this->load = 0;

	this->buffer[QUEUE_DEPTH]   = 0xBAAD; // Third to last element in the buffer
	this->buffer[QUEUE_DEPTH+1] = 0xBAAD; // Second to Last element in the buffer
	this->buffer[QUEUE_DEPTH+2] = 0xBAAD; // Last element in the buffer

}

// Enqueueing function
bool enqueue(Queue* this, int16_t data){

	int16_t location;


	if(this->load >= QUEUE_DEPTH){
		return false;
	}

	/*
	uint16_t irqs = __get_PRIMASK();
	if(irqs==0){
		__disable_irq();
	}
	*/


	// Adjusting that location if it wraps around the end of the buffer
	if (this->head + this->load >= QUEUE_DEPTH){
		// Wrap around
		location = (this->head + this->load) - QUEUE_DEPTH;
	}
	else {
		// No wrap around
		location = this->head + this->load;
	}


	this->buffer[location] = data;
	this->load++;


	if(this->load >= QUEUE_DEPTH){
		this->load = 0;
	}


	/*
	if(irqs==0){
		__enable_irq();
	}
	*/

	return true;


}



// Dequeueing function
bool dequeue(Queue* this, int16_t* data){
	if(this->load == 0){
		return false;
	}

	/*
	uint32_t irqs = __get_PRIMASK();
	if(irqs==0){
		__disable_irq();
	}
	*/

	*data = this->buffer[this->head];
	this->head++;


	if(this->head >= QUEUE_DEPTH){
		this->head = 0;
	}

	this->load--;

	/*
	if(irqs==0){
		__enable_irq();
	}
	*/

	return true;
}



// This function monitors all of the queues to ensure there is no buffer overflow by checking all of the bad food values
// the Queues get reinitialized if there is corruption
void monitor_queues(){


	// Iterating through the last 3 ("hidden") values of the M1_setup_q queue buffer
	for (volatile uint8_t i = 0; i<=2; i++){
		if(Mechanum_q.buffer[QUEUE_DEPTH+i] != 0xBAAD){
			init_queue(&Mechanum_q);
		}
	}


	// Iterating through the last 3 ("hidden") values of the M1_data_q queue buffer
	for (volatile uint8_t i = 0; i<=2; i++){
		if(TX_q.buffer[QUEUE_DEPTH+i] != 0xBAAD){
			init_queue(&TX_q);
		}
	}
}

// Simple function that inits all the queues
void init_all_queues(){

	init_queue(&Mechanum_q);	// Mode1 Setup Queue
	init_queue(&TX_q);		// Mode1 Data Queue

	return;
}



