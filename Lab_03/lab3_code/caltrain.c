#include <pthread.h>
#include <stdio.h>
#include "caltrain.h"

void
station_init(struct station *station)
{
	station->free_seat = 0;
	(*station).pessangers_riding_count = 0;
	(*station).pessangers_waiting_count = 0;
	pthread_mutex_init(&station->mutex, NULL);
	pthread_cond_init(&station->train_arrived, NULL);
	pthread_cond_init(&station->pessangers_on_board, NULL);
}

void
station_load_train(struct station *station, int count)
{
	if(!count || !(station->pessangers_waiting_count)) return;
	station->free_seat = count;
	
	pthread_mutex_lock(&station->mutex);
	pthread_cond_broadcast(&station->train_arrived);
	pthread_cond_wait(&station->pessangers_on_board, &station->mutex);
	pthread_mutex_unlock(&station->mutex);
}

void
station_wait_for_train(struct station *station)
{
	pthread_mutex_lock(&station->mutex);
	station->pessangers_waiting_count++;
	while((*station).free_seat == 0){
		pthread_cond_wait(&station->train_arrived, &station->mutex);
	}
	(*station).free_seat--;
	station->pessangers_waiting_count--;
	(*station).pessangers_riding_count++;
	pthread_mutex_unlock(&station->mutex);
}

void
station_on_board(struct station *station)
{
	pthread_mutex_lock(&station->mutex);
	station->pessangers_riding_count--;
	if(station->pessangers_riding_count == 0 && 
		(station->free_seat == 0 || station->pessangers_waiting_count == 0))
		pthread_cond_signal(&station->pessangers_on_board);
	pthread_mutex_unlock(&station->mutex);
}
