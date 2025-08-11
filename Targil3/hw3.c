#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define N 10 // Max patients in the clinic
#define SOFA_CAPACITY 4 // Max seats on the sofa
#define HYGIENISTS 3 // Number of hygienists
#define TOTAL_PATIENTS (N + 2) // Total patients trying to enter

// Semaphores
sem_t sofa_space;          // Number of seats on the sofa
sem_t hygienist_ready;     // Hygienists available for treatment
sem_t hygienist_treatment_done[N+2];     // Indicates treatment is done for patient[i], 0<=i<=N+1
sem_t payment_ready;       // Indicates payment is ready
sem_t payment_accepted;    // Indicates payment is accepted by hygienist
sem_t payment_mutex;    // Mutex to ensure payment for only one patient at a time
sem_t all_patients_entered; // Synchronization barrier for all patients
sem_t sofa_queue_access;   // Semaphore to protect sofa queue
sem_t completed_mutex;     // Mutex to protect completed count
sem_t patients_on_sofa;    // Tracks patients waiting for treatment on the sofa
sem_t patient_ready_for_treatment; // Indicates patient is ready for treatment
sem_t sofa_queue_treatment; // Semaphore to protect sofa queue
sem_t leave_clinic; // Semaphore to protect leave clinic
sem_t log_mutex; // Semaphore to protect log entry

// Global variables
int entered_patients = 0;   // Number of patients who have entered the clinic
int completed_patients = 0; // Number of patients who completed the process
int total_attempted_patients = 0; // Tracks total patients who attempted to enter
int sofa_available = 4; // Number of available spots on the sofa

int log_file; // Log file descriptor

// Patient structure for linked list
typedef struct Patient {
	int id;
	struct Patient *next;
} Patient;

// Head pointers for queues
Patient *sofa_queue = NULL;   // Queue for patients on the sofa
Patient *clinic_queue = NULL; // Queue for patients in the clinic
Patient *clinic_extra_queue = NULL; // Queue for patients outside of the clinic
Patient *treatment_queue = NULL; // Queue for patients that are in treatment

// Functions to manage linked lists in FIFO style
void enqueue(Patient **head, int id) {
    /* The function adds to Patient list the Patient node with recieved id to
    the end of the list*/
	Patient *new_patient = (Patient *)malloc(sizeof(Patient)); // Creates a new node
	new_patient->id = id;
	new_patient->next = NULL;

	if (*head == NULL) { // If the list is empty, it adds it to the head
		*head = new_patient;
	} else { // Otherwise, it adds it to the end of the list
		Patient *temp = *head;
		while (temp->next != NULL) {
			temp = temp->next;
		}
		temp->next = new_patient;
	}
}

int dequeue(Patient **head) {
    // Removes the first node of the list (the first node in the queue order), and returns the id of the first Patient node in the list
	if (*head==NULL) { // If queue is empty it returns -1
		return -1;
	}
	// Otherwise, removes the first Patient in the queue and returns their id
	Patient *temp = *head;
	int id = temp->id;
	*head = temp->next;
	free(temp);
	return id;
}

void print_queue(Patient *head) { 
    /*This function is for printing the queue, it wasn't requested it was function mainly for debbuging*/
	Patient *temp = head;
	while (temp != NULL) {
		printf("%d ", temp->id);
		temp = temp->next;
	}
	printf("\n");
}

int is_in_queue(Patient *head, int id) {
    /*The function checks if there is a Patient with id id in the list, if so it returns 1 otherwise 0*/
	Patient *temp = head;
	while (temp != NULL) {
		if (temp->id == id) 
		    return 1;
		temp = temp->next;
	}
	return 0;
}

void log_entry(char str[]){
    /* The function gets a string and writes it to the log_file*/
    // To ensure there is no race between the threads it has mutex to protect it
    sem_wait(&log_mutex);
	if(write(log_file,str,strlen(str))==-1){ // Exits the program if it failed to write
		perror("Failed to write in log file");
		exit(EXIT_FAILURE);
	}
    sem_post(&log_mutex);
}

void *patient(void *ptr) {
    /* The function of the patients threads, it manages the order of the patients
    in the clinic and write their actions to the log. The order of thing is according to:
    All N patients must enter the clinic before a patient can be seated on the sofa, 
    if a patient N+1 tries to enter it doesn't allow them. According to the entrance
    queue 4 patients will be seated on the sofa, and the rest will stay standing. Once
    the sofa is full (capacity is 4 patients), the first 3 according to the order of sitting
    will get treatement. When they get a treatment, they are not sitting on the sofa so the 
    next patients according to the entrance queue can sit on the sofa. After a patient was
    treated by a hygienist they will pay for the treatment and only after the hygienist got
    the payment the patient will be allowed to leave the clinic. When a patient leaves the
    clinic they are added to the extra queue (The queue of the patients outside the clinic),
    and the first patient from the extra queue is added to the clinic queue,
    in other words they can enter the clinic since a patient left.*/
	int id = *(int *)ptr; // id of the patient
	int temp_id;
	int extra_id;
	char str[100]; // String that will be used to put the actions of the patient into the log file 
	while (1) { // Infinite loop for patient actions
		// Check if the patient can enter the clinic
		// For the first entrance, it ensures that N patients have entered and the rest couldn't enter
		sem_wait(&completed_mutex);
		total_attempted_patients++;
		if (entered_patients < N) {
			entered_patients++;
			// Writes to the log that the patient entered the clinic
			sprintf(str, "I'm Patient #%d, I got into the clinic\n", id);
			log_entry(str);
			enqueue(&clinic_queue, id); // Adds the patient id to the clinic queue, the list that has the patients that entered to the clinic
		} else { // The clinic is full, it will try to add the patient to the extra queue
			if (!is_in_queue(clinic_extra_queue, id) && !is_in_queue(clinic_queue, id) && !is_in_queue(sofa_queue, id)&& !is_in_queue(treatment_queue,id)){
			    // Only if a patient, isn't in any of the queues it will add them to the extra queue
			    // Writes to the log that the patient couldn't enter the clinic 
				sprintf(str, "I'm Patient #%d, I cannot enter the clinic\n", id);
				log_entry(str);
				enqueue(&clinic_extra_queue, id); // Adds the patient id to the extra queue, the queue of the patients that are outside from the clinic
			}
		}
		if (total_attempted_patients == N+2) { 
			for (int i = 0; i <= entered_patients; i++) {
				sem_post(&all_patients_entered);
			}
		}
		sem_post(&completed_mutex);
		// Wait until all patients have attempted to enter
		sem_wait(&all_patients_entered);
		
		// Only when there are N patients in the clinic it allows the first 4 to be seated on the sofa
		// Sit on the sofa, the sofa has only 4 places, the sitting order to the sofa is according to the entrance order
		sem_wait(&sofa_queue_access);
		sem_wait(&sofa_space); // Wait for a free sofa space
		while (sofa_available > 0){ // Adds patients according to the entrance order until there is no more space on the sofa
			temp_id = dequeue(&clinic_queue);
			if (temp_id == -1) break; // Exits the loop if the clinic queue is empty
			enqueue(&sofa_queue, temp_id); // Add to sofa queue
				// Writes to the log that the patient is sittting on the sofa
			sprintf(str, "I'm Patient #%d, I'm sitting on the sofa\n", temp_id);
			log_entry(str);
			sofa_available--; // Decresease the amount of available space on the sofa
		}
		sem_post(&sofa_queue_access);
		sem_post(&patients_on_sofa); // Signal that a patient is ready for treatment
		
		// Wait for a hygienist
		sem_wait(&hygienist_ready);
		
		// Treatment, happens to the patient only if they sat on the sofa
		sem_wait(&sofa_queue_treatment);
		int treatment_id = dequeue(&sofa_queue); // Get the next patient for treatment
		if (treatment_id == -1) // if sofa queue is empty, it will go to the next iteration of the while
		{
			sem_post(&sofa_queue_treatment);
			continue;
		}
		enqueue(&treatment_queue,treatment_id); // Adds them to the treatment queue
		sofa_available++; // Increases space for another patient to sit on the sofa
		sem_post(&sofa_queue_treatment);
		// Writes to the log that the patient is getting treatment
		sprintf(str, "I'm Patient #%d, I'm getting treatment\n", treatment_id);
		log_entry(str);
		// Signal that the patient is ready for treatment
		sem_post(&patient_ready_for_treatment);
		// Wait for treatment to finish
		sem_wait(&hygienist_treatment_done[treatment_id-1]);
		
		// Payment, to ensure there is only one patient paying there is a mutex
		sem_wait(&payment_mutex);
		// Writes to the log that the patient is paying
		sprintf(str, "I'm Patient #%d, I'm paying now\n", treatment_id);
		log_entry(str);
		sem_post(&payment_ready); // Notify hygienist that payment is ready
		sem_wait(&payment_accepted); // Wait for hygienist to get payment
		sem_post(&payment_mutex);
		
		// Leave the clinic, gets here only if hygienist got the payment 
		sem_post(&sofa_space);  // Free a sofa space
		sem_wait(&completed_mutex);
		completed_patients++;
		extra_id = dequeue(&clinic_extra_queue); // gets the first patient from the list of those outside the clinic, if the list wasn't empty we will add that patient to the clinic list
		sem_post(&completed_mutex);
		sem_wait(&leave_clinic);
		// Writes to the log that the patient has left the clinic
		sprintf(str, "I'm Patient #%d, I left the clinic\n", treatment_id);
		log_entry(str);
		enqueue(&clinic_extra_queue,treatment_id); // Puts the Patient who left in the list of those outside the clinic
		if (extra_id != -1&&!is_in_queue(clinic_queue,extra_id)) // Adds the extra_id to the clinic list, if the extra list isn't empty of course
		{
			enqueue(&clinic_queue, extra_id);
			// Writes to the log that the patient extra_id has entered the clinic the clinic
			sprintf(str, "I'm Patient #%d, I got into the clinic\n", extra_id);
			log_entry(str);
			sem_post(&all_patients_entered); // Allows the loop to continue working
		}
		sem_post(&leave_clinic);
	}
}

void *hygienist(void *ptr) {
    /* Function to manage hygienists, a hygienist waits until there are people sitting
    in the sofa in the clinic and only when a patient says their getting a treatment,
    they start to work. The hygienist does 2 things, they give treatment and get payment
    , of course the patient needs to declare first that they are getting a treatment
    and only then the hygienist start to give a treatment. Only after a treatment 
    is done, the patient can pay for the treatment, and because there is only one
    regiser no other patient can pay in the same time, only after the hygienist
    got the payment from the patient he is allowed to leave the clinic, the actions
    of each hygienist are written to the log file*/
	int id = *(int *)ptr; // Number of the hygienist
	int treatment_id;
    char str[100]; // String that will be used to put the actions of the hygenist into the log file
	while (1) { // Infinite loop for hygienist actions
		// Wait for a patient to be ready on the sofa
		sem_wait(&patients_on_sofa);
		// The hygienist is ready to treat
		sem_post(&hygienist_ready);
        
		// Wait for the patient to be ready for treatment
		sem_wait(&patient_ready_for_treatment);

		// Simulate treatment and payment
		treatment_id=dequeue(&treatment_queue); // Gets the patient id from the treatment queue
		if(treatment_id!=-1){ // If the queue wasn't empty
		    // Writes to the log that the hygienist is working
    		sprintf(str, "I'm Dental Hygienist #%d, I'm working now\n", id);
            log_entry(str);
    		// Finished working on treatment_id, now the patient needs to pay for the treatment
    		sem_post(&hygienist_treatment_done[treatment_id-1]);
    
    		// Wait for payment
    		sem_wait(&payment_ready);
    		// Writes to the log that the hygienist is received a payment
    		sprintf(str, "I'm Dental Hygienist #%d, I'm receiving payment\n", id);
    		log_entry(str);
    		// Signals payment accepted, allowing the patient with treatment_id to leave
    		sem_post(&payment_accepted);
		}
	}

}


int main() {
    /*The main function, simulates how a clinic works using N+2 patients and 3 hygienists,
    the threads call to their functions with their serial numbers. The functions 
    of hygenist and patient run Infinitly, so in order to end the program press ctrl+c*/
    // Initialize Threads
	pthread_t patients[N+2];
	pthread_t hygienists[3];
	// Initialize variables
    int arrayP[N]; // Serial numbers for patients
    int arrayH[3]={1,2,3}; // Serial numbers for hygienists
    int i;
	// Initialize semaphores
	sem_init(&sofa_space, 0, SOFA_CAPACITY); // 4 seats on the sofa
	sem_init(&hygienist_ready, 0, 0);     // No hygienists ready initially
	for(i=0;i<N+2;i++)
	    sem_init(&hygienist_treatment_done[i], 0, 0); // Semaphore for each of the patients if their treatment is done
	sem_init(&payment_ready, 0, 0);       // Payment readiness
	sem_init(&payment_accepted, 0, 0);    // Payment acceptance
	sem_init(&payment_mutex, 0, 1);
	sem_init(&all_patients_entered, 0, 0); // Synchronization barrier for all patients
	sem_init(&sofa_queue_access, 0, 1);   // Semaphore to protect sofa queue
	sem_init(&completed_mutex, 0, 1);     // Mutex to protect completed count
	sem_init(&patients_on_sofa, 0, 0);    // Tracks patients waiting for treatment on the sofa
	sem_init(&patient_ready_for_treatment, 0, 0); // No patients ready initially
	sem_init(&sofa_queue_treatment, 0, 1); // Semaphore to protect sofa queues
	sem_init(&leave_clinic, 0, 1); // Semaphore to protect leave clinic
    sem_init(&log_mutex,0,1); // Semaphore to protect Log entry
    	
    // Open log file
    log_file = open("Libr.txt", O_WRONLY|O_CREAT|O_APPEND|O_TRUNC,0664);
	if (log_file==-1) { // Exits the program if failed
		perror("Failed to open log file");
		exit(EXIT_FAILURE);
	}
	
	// Create hygienist threads
	for (i = 0; i < 3; i++) {
		arrayH[i]=i+1;
		if(pthread_create(&hygienists[i], NULL, hygienist, (void*)&arrayH[i])==-1){ // Exits the program if failed
		    perror("Thread creation failed");
		    exit(EXIT_FAILURE);
		}
	}

	// Create patient threads
	for (i = 0; i < N+2; i++) {
		arrayP[i]=i+1;
		if(pthread_create(&patients[i], NULL, patient, (void*)&arrayP[i])==-1){ // Exits the program if failed
		    perror("Thread creation failed");
		    exit(EXIT_FAILURE);
		}
	}

	// Untill all the threads finish their functions the main function won't continue (Which never happaens since the thread function run Infinitly)
	for (int i = 0; i < N+2; i++) {
		pthread_join(patients[i], NULL);
	}
	for (int i = 0; i < 3; i++) {
		pthread_join(hygienists[i], NULL);
	}
	// Closes the handle for log_file, never reaches here because the program ends with ctrl+c
    close(log_file);
	return 0;
}