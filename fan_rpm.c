/**************************
*     RPM for Fan
**************************/
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "gpiolib.h"

#define CLOCKID CLOCK_REALTIME
#define SIG SIGRTMIN
#define err_exit(msg) do { perror(msg); exit(EXIT_FAILURE);} while (0)

#define GPIO(n) n

/* MX8MP_IOMUXC_I2C3_SCL__GPIO5_IO18 */
#define RPM_READ GPIO(146)

int rpm_read = RPM_READ;

pthread_mutex_t mutex;

void *(*start_routine[0])(void *);
int paules_cnt;

void alarm_handler (int signo)
{
	int fan_rpm;
	int fan_rpm_tmp;

	pthread_mutex_lock(&mutex);
	fan_rpm_tmp = paules_cnt;
	paules_cnt = 0;
	pthread_mutex_unlock(&mutex);

	fan_rpm = fan_rpm_tmp / 4 / 2 * 60;
	printf ("fan_rpm %d\n", fan_rpm);
}

int start_timer()
{
	struct itimerval timer_val;
	int ret;

	signal (SIGALRM, alarm_handler);

	timer_val.it_value.tv_sec = 2;
	timer_val.it_value.tv_usec = 0;
	timer_val.it_interval.tv_sec = 2;
	timer_val.it_interval.tv_usec = 0;
	ret = setitimer (ITIMER_REAL, &timer_val, NULL);
	if (ret) {
		perror ("setitimer");
		return 0;
	}

	return 0;
}

int stop_timer()
{
	struct itimerval timer_val = {0};
	int ret;

	stop_timer();
	ret = setitimer (ITIMER_REAL, &timer_val, NULL);
	if (ret) {
		perror ("setitimer");
		return -1;
	}

	return 0;
}

void *thr_rpm_read (void *rpm_read)
{
	int *rpm_read_pin;
	int val = 0;

	rpm_read_pin = rpm_read;

	pthread_mutex_lock (&mutex);
	gpio_export(*rpm_read_pin);
	gpio_direction(*rpm_read_pin, GPIO_IN);
	gpio_setedge(*rpm_read_pin, RISI_EN, FALL_DIS);
	pthread_mutex_unlock (&mutex);

	pthread_mutex_lock (&mutex);
	val = gpio_select(*rpm_read_pin);
	pthread_mutex_unlock (&mutex);

	gpio_setedge(*rpm_read_pin, RISI_EN, FALL_EN);
	start_timer();

	while (1) {
		val = gpio_select(*rpm_read_pin);
		if (val == 1)
			paules_cnt++;
		val = 0;
	};

	gpio_setedge(*rpm_read_pin, RISI_DIS, FALL_DIS);
	gpio_unexport(*rpm_read_pin);

	return NULL;
}


int main(int argc, char **argv)
{
	pthread_t thread[0];
	pthread_mutex_init (&mutex, NULL);

	paules_cnt = 0;
	start_routine[0] = thr_rpm_read;
	pthread_create (&thread[0], NULL, (*start_routine[0]), &rpm_read);

	sleep(2);
	return 0;
}
