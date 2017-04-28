// Two glboal variables to store address of front and rear nodes. 
struct Node* front = NULL;
struct Node* rear = NULL;
unsigned long itemCount = 0;

// To Enqueue an integer
void Enqueue(char x) {
	struct Node* temp = 
		(struct Node*)malloc(sizeof(struct Node));
	if (temp == NULL)
	{
	    perror("malloc error\n");
	    exit(0);
	}
	itemCount++;
	temp->data =x;
	temp->next = NULL;
	if(front == NULL && rear == NULL){
		front = rear = temp;
	}
	else
	{
	    rear->next = temp;
	    rear = temp;
	}
}

// To Dequeue an integer.
void Dequeue() {
	struct Node* temp = front;
	if(front == NULL) {
		printf("Queue is Empty\n");
		return;
	}
	if(front == rear) {
		front = rear = NULL;
	}
	else {
		front = front->next;
	}
	free(temp);
}

int Front() {
	if(front == NULL) {
		printf("Queue is empty\n");
		return;
	}
	return front->data;
}

int QueueIsEmpty() {
    return (front == NULL)?1:0;
}

void PrintQueue() {
	struct Node* temp = front;
	while(temp != NULL) {
		printf("%c ",temp->data);
		temp = temp->next;
	}
	printf("\n");
}
