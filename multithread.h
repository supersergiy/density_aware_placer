#pragma once

#include "omp.h"
#ifdef SERGIY_IS_NOT_BITCH
class ReaderWriterLock{
	omp_lock_t mutex, wrt;
	int readcount;
public:
	ReaderWriterLock()
	{
		readcount = 0;
		omp_init_lock(&wrt);
		omp_init_lock(&mutex);
	}

	~ReaderWriterLock()
	{
		omp_destroy_lock(&wrt);
		omp_destroy_lock(&mutex);
	}
	void enterReader(void)
	{
		omp_set_lock(&mutex);
		readcount++;
		if (readcount == 1){
			omp_set_lock(&wrt);
		}
		omp_unset_lock(&mutex);
	}
	/* ************************************************** */
	void exitReader(void)
	{
		omp_set_lock(&mutex);
		readcount--;
		if (readcount == 0){
			omp_unset_lock(&wrt);
		}
		omp_unset_lock(&mutex);
	}
	/* ************************************************** */
	void enterWriter(void)
	{
		omp_set_lock(&wrt);
	}
	/* ************************************************** */
	void exitWriter(void)
	{
		omp_unset_lock(&wrt);

	}
};
#endif
