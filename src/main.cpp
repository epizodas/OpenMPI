#include "../include/openmpi-x86_64/mpi.h"
// #include <mpi.h>
#include "io.hpp"
#include "object.hpp"
#include "queue.hpp"
#include <vector>

using namespace std;

enum tags {
  MAIN_AND_DATA = 10,
  DATA_ACCEPT_MAIN = 11,
  DATA_REFUSE_MAIN = 12,
  MAIN_TO_DATA_COUNT = 13,
  MAIN_TO_RESULT_COUNT = 14,
  DATA_AND_WORKER = 20,
  WORKER_AND_RESULT = 30,
  RESULT_AND_MAIN = 40,
  WORKER_WANTS_DATA = 100,
  JOB_DONE = 200,
};

// Process rank constants
const int MAIN_THREAD = 0;
const int DATA_THREAD = 1;
const int RESULT_THREAD = 2;

const int criteria = 500;

static void send_object(Object &data, int dest, int tag) {
  MPI_Send(&data, sizeof(Object), MPI_CHAR, dest, tag, MPI_COMM_WORLD);
}

static Object get_object(int source, int tag) {
  Object data;
  MPI_Recv(&data, sizeof(Object), MPI_CHAR, source, tag, MPI_COMM_WORLD,
           MPI_STATUS_IGNORE);
  return data;
}

void worker(int worker_id) {

  while (1) {

    int mesg = 0;
    MPI_Send(&mesg, 1, MPI_INT, DATA_THREAD, DATA_AND_WORKER, MPI_COMM_WORLD);
    cout << "Worker " << worker_id << " - establishing communication" << endl;

    int should_die = 0;
    MPI_Recv(&should_die, 1, MPI_INT, DATA_THREAD, DATA_AND_WORKER,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    cout << "Worker " << worker_id << " - asking if should die" << endl;

    if (should_die == 1) {
      int mesg = 0;
      MPI_Send(&mesg, 1, MPI_INT, RESULT_THREAD, JOB_DONE, MPI_COMM_WORLD);
      cout << "Worker " << worker_id << " - dead" << endl;
      break;
    }

    Object data = get_object(DATA_THREAD, DATA_AND_WORKER);
    cout << "Worker " << worker_id << " - got data" << endl;

    data.computeStockValue();

    if (data.getStockValue() > criteria) {
      send_object(data, RESULT_THREAD, WORKER_AND_RESULT);
      cout << "Worker " << worker_id << " - found correct " << data.to_string()
           << endl;
    }
  }
}

void data_thread() {
  int total_items = 0;
  MPI_Recv(&total_items, 1, MPI_INT, MAIN_THREAD, MAIN_TO_DATA_COUNT,
           MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  int capacity = (total_items > 1) ? (total_items / 2) : 1;
  cout << "Data - initializing monitor with capacity: " << capacity << endl;
  Queue monitor = Queue(capacity);
  int no_more_data = 0;

  int process_amount;
  MPI_Comm_size(MPI_COMM_WORLD, &process_amount);
  int worker_amount = process_amount - 3;
  int dead_worker_amount = 0;

  while (1) {
    int flag = 0;
    MPI_Status status;
    MPI_Iprobe(MAIN_THREAD, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
    if (flag && status.MPI_TAG == MAIN_AND_DATA && !monitor.isFull()) {
      Object data = get_object(MAIN_THREAD, MAIN_AND_DATA);
      monitor.push(data);
      cout << "Data - receiving data. Queue size: " << monitor.getSize()
           << endl;
    }
    if (flag && status.MPI_TAG == JOB_DONE) {
      MPI_Recv(&no_more_data, 1, MPI_INT, MAIN_THREAD, JOB_DONE, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      cout << "Data - no more data will come. "
           << "Current data amount: " << monitor.getSize() << endl;
    }
    MPI_Iprobe(MPI_ANY_SOURCE, DATA_AND_WORKER, MPI_COMM_WORLD, &flag, &status);
    if (flag && status.MPI_TAG == DATA_AND_WORKER && monitor.isEmpty() &&
        no_more_data == 1) {
      int mesg = 0;
      MPI_Recv(&mesg, 1, MPI_INT, status.MPI_SOURCE, DATA_AND_WORKER,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      cout << "Data - getting communication" << endl;
      int should_die = 1;
      MPI_Send(&should_die, 1, MPI_INT, status.MPI_SOURCE, DATA_AND_WORKER,
               MPI_COMM_WORLD);
      cout << "Data - worker should die" << endl;
      dead_worker_amount++;
    }
    if (flag && status.MPI_TAG == DATA_AND_WORKER && !monitor.isEmpty()) {
      int mesg = 0;
      MPI_Recv(&mesg, 1, MPI_INT, status.MPI_SOURCE, DATA_AND_WORKER,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      cout << "Data - getting communication" << endl;
      int should_die = 0;
      MPI_Send(&should_die, 1, MPI_INT, status.MPI_SOURCE, DATA_AND_WORKER,
               MPI_COMM_WORLD);
      cout << "Data - worker should not die" << endl;
      Object obj_preprocessed = monitor.pop();
      send_object(obj_preprocessed, status.MPI_SOURCE, DATA_AND_WORKER);
      cout << "Data - sending data to worker: " << obj_preprocessed.to_string()
           << ". Queue size: " << monitor.getSize() << endl;
    }
    if (dead_worker_amount == worker_amount) {
      cout << "Data - dead" << endl;
      break;
    }
  }
}

void result_thread() {
  int total_items = 0;
  MPI_Recv(&total_items, 1, MPI_INT, MAIN_THREAD, MAIN_TO_RESULT_COUNT,
           MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  int capacity = (total_items > 0) ? total_items : 1;
  cout << "Result - initializing monitor with capacity: " << capacity << endl;
  Queue monitor = Queue(capacity);

  int process_amount;
  MPI_Comm_size(MPI_COMM_WORLD, &process_amount);
  int worker_amount = process_amount - 3;
  int dead_worker_amount = 0;

  while (1) {
    MPI_Status status;
    MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    if (status.MPI_TAG == WORKER_AND_RESULT) {
      Object data = get_object(status.MPI_SOURCE, WORKER_AND_RESULT);
      monitor.insertSorted(data);
      cout << "Result - got data from worker" << endl;
    }
    if (status.MPI_TAG == JOB_DONE) {
      int mesg;
      MPI_Recv(&mesg, 1, MPI_INT, status.MPI_SOURCE, JOB_DONE, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      dead_worker_amount++;
    }
    if(dead_worker_amount == worker_amount){
        cout << "Result - all workers dead" << endl;
        break;
      }
  }
  cout << "Result - all workers finished, sending data to main..." << endl;
  int amount = monitor.getSize();
  MPI_Send(&amount, 1, MPI_INT, MAIN_THREAD, RESULT_AND_MAIN, MPI_COMM_WORLD);

  while (!monitor.isEmpty()) {
    Object data = monitor.pop();
    send_object(data, MAIN_THREAD, RESULT_AND_MAIN);
  }
}

int main(int argc, char **argv) {
  MPI_Init(&argc, &argv);

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (rank == MAIN_THREAD) {
    vector<Object> data = read();
    int total_items = static_cast<int>(data.size());
    MPI_Send(&total_items, 1, MPI_INT, DATA_THREAD, MAIN_TO_DATA_COUNT, MPI_COMM_WORLD);
    MPI_Send(&total_items, 1, MPI_INT, RESULT_THREAD, MAIN_TO_RESULT_COUNT, MPI_COMM_WORLD);
    for (int i = 0; i < data.size(); i++) {
      send_object(data[i], DATA_THREAD, MAIN_AND_DATA);
      cout << "Main - sending data " << data[i].to_string() << endl;
    }

    int no_more_data = 1;
    MPI_Send(&no_more_data, 1, MPI_INT, DATA_THREAD, JOB_DONE, MPI_COMM_WORLD);
    int data_amount;
    MPI_Recv(&data_amount, 1, MPI_INT, RESULT_THREAD, RESULT_AND_MAIN,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    Queue results = Queue(data.size());
    for (int i = 0; i < data_amount; i++) {
      Object obj = get_object(RESULT_THREAD, RESULT_AND_MAIN);
      results.push(obj);
    }
    write(results, "IFF-3-7_PadelskasE_L1_rez.txt");
  } else if (rank == DATA_THREAD) {
    data_thread();
  } else if (rank == RESULT_THREAD) {
    result_thread();
  } else {
    worker(rank);
  }

  MPI_Finalize();
  return 0;
}