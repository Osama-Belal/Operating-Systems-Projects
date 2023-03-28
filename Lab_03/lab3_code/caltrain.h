#include <pthread.h>

struct station {
	int free_seat;
	int pessangers_waiting_count;
	int pessangers_riding_count;
	pthread_mutex_t mutex;
	pthread_cond_t train_arrived;
	pthread_cond_t pessangers_on_board;
};

void station_init(struct station *station);

void station_load_train(struct station *station, int count);

void station_wait_for_train(struct station *station);

void station_on_board(struct station *station);