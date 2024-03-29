/*
 * tester.c
 *
 *  Created on: May 22, 2012
 *      Author: root
 */

#include <stdbool.h>
#include <stdio.h>
#include "lock.h"

#define PRINT_RESULT(test_num, result) \
	printf("Test number %d:  %s\n", test_num, result ? "Yeah!!" : "Fuck!!")

#define ARR_SIZE 8192

int array[ARR_SIZE];
int counter;
//rmww_lock lock;

void* work_on_array(void* temp_lock) {
	rmww_lock* lock = (rmww_lock*)temp_lock;
	int i;
	bool is_change = false;

	get_write_lock(lock);
	counter++;
	release_exclusive_lock(lock);

	for (i = 0; i < ARR_SIZE; ++i) {
		get_may_write_lock(lock);
		if (array[i] == (pthread_self() % ARR_SIZE)) {
			upgrade_may_write_lock(lock);
			array[i] = pthread_self();
			printf("Thread number: %d was here :)\n", array[i]);
			is_change = true;
			break;
		}
		release_shared_lock(lock);
	}

	if (is_change) {
		release_exclusive_lock(lock);
	}

	return NULL;
}

bool Test1() {
	int i;
	for (i = 0; i < ARR_SIZE; ++i) {
		array[i] = i;
	}

	rmww_init(&lock);

	pthread_t thread1;
	pthread_t thread2;
	pthread_t thread3;
	pthread_t thread4;
	pthread_t thread5;

	pthread_create(&thread1, NULL, work_on_array, &lock);
	pthread_create(&thread2, NULL, work_on_array, &lock);
	pthread_create(&thread3, NULL, work_on_array, &lock);
	pthread_create(&thread4, NULL, work_on_array, &lock);
	pthread_create(&thread5, NULL, work_on_array, &lock);

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	pthread_join(thread3, NULL);
	pthread_join(thread4, NULL);
	pthread_join(thread5, NULL);

	rmww_destroy(&lock);
	return (counter == 5);
}

void* work_on_array2(void* temp_lock) {
	rmww_lock* lock = (rmww_lock*)temp_lock;
	int i;
	int temp;
	for (i = 0; i < ARR_SIZE; ++i) {
		get_read_lock(lock);
		temp = array[i];
		release_shared_lock(lock);
	}

	return NULL;
}

void* work_on_array3(void* temp_lock) {
	rmww_lock* lock = (rmww_lock*)temp_lock;
	int i;
	for (i = 0; i < ARR_SIZE; ++i) {
		get_may_write_lock(lock);
		if ((i % 2) == 0) {
			upgrade_may_write_lock(lock);
			array[i] += 2;
			release_exclusive_lock(lock);
			continue;
		}
		release_shared_lock(lock);
	}

	return NULL;
}

void* work_on_array4(void* temp_lock) {
	rmww_lock* lock = (rmww_lock*)temp_lock;
	int i;
	for (i = ARR_SIZE - 1; i >= 0; i -= 2) {
		get_write_lock(lock);
		array[i] += 3;
		release_exclusive_lock(lock);
	}

	return NULL;
}

bool Test2() {
	int i;
	for (i = 0; i < ARR_SIZE; ++i) {
		array[i] = 0;
	}

	rmww_init(&lock);

	pthread_t thread1;
	pthread_t thread2;
	pthread_t thread3;
	pthread_t thread4;
	pthread_t thread5;

	pthread_create(&thread1, NULL, work_on_array2, &lock);
	pthread_create(&thread2, NULL, work_on_array3, &lock);
	pthread_create(&thread3, NULL, work_on_array3, &lock);
	pthread_create(&thread4, NULL, work_on_array4, &lock);
	pthread_create(&thread5, NULL, work_on_array4, &lock);

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	pthread_join(thread3, NULL);
	pthread_join(thread4, NULL);
	pthread_join(thread5, NULL);

	for (i = 0; i < ARR_SIZE; ++i) {
		if ( (((i % 2) == 0) && array[i] != 4) ||
			 (((i % 2) == 1) && array[i] != 6))
			return false;
	}

	rmww_destroy(&lock);
	return true;
}

//int main() {
//	PRINT_RESULT(1, Test1());
//	PRINT_RESULT(2, Test2());
//	int i = 0;
//	for(i = 0; i < ARR_SIZE - 4; i += 5) {
//		printf("%d  %d  %d  %d  %d\n", array[i], array[i + 1], array[i + 2],
//				array[i + 3], array[i + 4]);
//	}
//	return 1;
//}
