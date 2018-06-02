#ifndef __WORK_JOB_H__
#define __WORK_JOB_H__

typedef enum _WORKER_JOB_TYPE
{
	BROWSER_WEB_PAGE, 
	MAX_WORKER_JOB_TYPE, 
} WORKER_JOB_TYPE, *PWORKER_JOB_TYPE; 

typedef struct _WORKER_JOB
{
	LIST_ENTRY entry; 
	//WORKER_JOB_TYPE work_type; 
	PVOID param; 
	LPTHREAD_START_ROUTINE work_routine; 
} WORKER_JOB, *PWORKER_JOB; 

//#define CONTAINING_RECORD(_address, _type, _field) ((_type *)( (PCHAR)(_address) - (ULONG_PTR)(&((_type*)0)->_field)))

#define WAIT_WORK_JOB 0x00000001

typedef struct _JOB_WORKER
{
	HANDLE worker; 
	BOOLEAN stop_working; 
	HANDLE work_notify; 
	ULONG working_state; 

	HANDLE work_lock; 
	LIST_ENTRY works; 
}JOB_WORKER, *PJOB_WORKER; 

extern JOB_WORKER job_worker; 

ULONG CALLBACK worker_thread( PVOID param ); 
LRESULT WINAPI start_job_worker( PJOB_WORKER worker ); 
LRESULT WINAPI stop_job_worker( PJOB_WORKER worker, ULONG wait_time ); 
LRESULT WINAPI add_job_to_worker( PJOB_WORKER worker, PVOID param, LPTHREAD_START_ROUTINE work_routine ); 

INLINE LRESULT WINAPI add_job_to_default_worker( PVOID param, LPTHREAD_START_ROUTINE work_routine )
{
	return add_job_to_worker( &job_worker, param, work_routine ); 
}

LRESULT WINAPI check_all_work_done( PJOB_WORKER worker ); 
LRESULT WINAPI check_job_worker_exit( PJOB_WORKER worker ); 
INLINE LRESULT WINAPI worker_job_end( PJOB_WORKER worker )
{
	ASSERT( NULL !=worker ); 
	//ASSERT( FALSE ); 
	worker->stop_working = TRUE; 
	worker->working_state = 0; 

	ASSERT( NULL != worker->work_notify ); 
	SetEvent( worker->work_notify ); 
	return S_OK ;
}

INLINE LRESULT WINAPI check_woker_job_exist( PJOB_WORKER worker )
{
	LRESULT ret = ERROR_SUCCESS; 
	ASSERT( NULL != worker->work_lock ); 

	lock_mutex( worker->work_lock ); 

	if( TRUE == IsListEmpty( &worker->works ) )
	{
		ret = ERROR_INVALID_PARAMETER; 
	}

	unlock_mutex( worker->work_lock ); 

	return ret; 
}

#endif //__WORK_JOB_H__