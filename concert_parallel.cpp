/*
Name: Shreevathsa Gorur Prashanth
SRN: PES1UG22CS568
Subject: HP Pthreads Assignment
*/

#include <fstream>
#include <iostream>
#include <pthread.h>
#include <queue>
#include <unistd.h>

#include <cstdlib>
#include <ctime>
#include <vector>

#define MAX_BUFFER_SIZE 5
#define NUM_THREADS 5

static volatile int available_tickets;
static volatile bool processing_done;

static int num_agents;

typedef struct customer_info {
  std::string customer_name;
  int requested_tickets;
} Customer;

std::queue<Customer> request_queue;

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ticket_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond_add = PTHREAD_COND_INITIALIZER;
pthread_cond_t queue_cond_remove = PTHREAD_COND_INITIALIZER;

void *process_ticket_request(void *arg) {
  for (;;) {
    pthread_mutex_lock(&queue_mutex);

    while (request_queue.empty() && !processing_done)
      pthread_cond_wait(&queue_cond_add, &queue_mutex);

    if (request_queue.empty() && processing_done) {
      pthread_mutex_unlock(&queue_mutex);
      break;
    }

    Customer current_request = request_queue.front();
    request_queue.pop();
    pthread_mutex_unlock(&queue_mutex);
    pthread_cond_signal(&queue_cond_remove);

    int tickets_needed = current_request.requested_tickets;
    std::string customer = current_request.customer_name;

    pthread_mutex_lock(&ticket_mutex);
    if (available_tickets >= tickets_needed) {
      available_tickets -= tickets_needed;
      std::cout << "Customer " << customer << " given " << tickets_needed
                << " tickets\n";
    } else if (available_tickets <= 0) {
      std::cout << "Customer " << customer << " given 0 tickets\n";
    } else {
      int tickets_left = tickets_needed - available_tickets;
      std::cout << "Customer " << customer << " given " << available_tickets
                << " tickets\n";
      available_tickets = 0;
    }

    pthread_mutex_unlock(&ticket_mutex);
  }

  return NULL;
}

int main(void) {
  std::ifstream input_file("sample_input.txt");

  if (!input_file) {
    std::cerr << "Error: Could not open input.txt\n";
    exit(EXIT_FAILURE);
  }

  int initial_ticket_count;
  input_file >> initial_ticket_count;
  available_tickets = reinterpret_cast<volatile int>(initial_ticket_count);

  processing_done = false;
  pthread_t thread_pool[NUM_THREADS];
  for (int i = 0; i < NUM_THREADS; i++)
    pthread_create(&thread_pool[i], NULL, process_ticket_request, NULL);

  std::string customer_name;
  int ticket_count;
  while (input_file >> customer_name >> ticket_count) {
    Customer customer = {customer_name, ticket_count};
    std::cout << "Customer " << customer_name << " requested " << ticket_count
              << " tickets\n";

    pthread_mutex_lock(&queue_mutex);
    while (request_queue.size() >= MAX_BUFFER_SIZE) {
      pthread_cond_wait(&queue_cond_remove, &queue_mutex);
    }

    request_queue.push(customer);
    pthread_mutex_unlock(&queue_mutex);
    pthread_cond_signal(&queue_cond_add);
  }
  input_file.close();

  pthread_mutex_lock(&queue_mutex);
  processing_done = true;
  pthread_mutex_unlock(&queue_mutex);
  pthread_cond_broadcast(&queue_cond_add);

  for (int i = 0; i < NUM_THREADS; i++)
    pthread_join(thread_pool[i], NULL);

  std::cout << "Remaining tickets: " << available_tickets << std::endl;

  pthread_mutex_destroy(&queue_mutex);
  pthread_mutex_destroy(&ticket_mutex);
  pthread_cond_destroy(&queue_cond_add);
  pthread_cond_destroy(&queue_cond_remove);
  return 0;
}
