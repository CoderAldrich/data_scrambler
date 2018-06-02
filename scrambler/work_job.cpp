#include "stdafx.h"
#include "work_job.h"

JOB_WORKER job_worker = { 0 }; 

ULONG CALLBACK worker_thread( PVOID param )
{
	LRESULT ret = ERROR_SUCCESS; 
	ULONG wait_ret; 
	PJOB_WORKER worker; 
	LIST_ENTRY *entry; 
	WORKER_JOB *job; 

	worker = ( PJOB_WORKER )param; 

	for( ; ; )
	{
		if( TRUE == worker->stop_working )
		{
			ASSERT( FALSE ); 
			break; 
		}

		worker->working_state = WAIT_WORK_JOB; 
		wait_ret = WaitForSingleObject( worker->work_notify, INFINITE ); 
		worker->working_state = 0; 

		if( wait_ret != WAIT_OBJECT_0 )
		{
			break; 
		}

		if( TRUE == worker->stop_working )
		{
			break; 
		}

		for( ; ; )
		{
			ret = lock_mutex( worker->work_lock ); 
			if( ret != ERROR_SUCCESS )
			{
				//break; 
			}

			entry = worker->works.Flink; 
			if( entry == &worker->works )
			{
				unlock_mutex( worker->work_lock ); 
				break; 
			}

			RemoveEntryList( entry ); 
			unlock_mutex( worker->work_lock ); 

			do 
			{
				if( TRUE == worker->stop_working )
				{
					break; 
				}

				job = CONTAINING_RECORD( entry, WORKER_JOB, entry ); 
				if( NULL != job->work_routine )
				{
					ret = job->work_routine( job->param ); 
					if( ret == ERROR_STOP_THREAD )
					{
						worker->stop_working = TRUE; 
					}

					break; 
				}

			}while( FALSE );

			free( job ); 

			if( TRUE == worker->stop_working )
			{
				//ASSERT( FALSE ); 
				break; 
			}
		} 
	} 

	return ret; 
}

LRESULT WINAPI start_job_worker( PJOB_WORKER worker ) 
{
	LRESULT ret = ERROR_SUCCESS; 

	do 
	{
		worker->work_notify = CreateEvent( NULL, FALSE, FALSE, NULL ); 
		if( NULL == worker->work_notify )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		worker->work_lock = CreateMutex( NULL, FALSE, NULL ); 
		if( NULL == worker->work_lock )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}

		InitializeListHead( &worker->works ); 
		worker->stop_working = FALSE; 
		worker->working_state = 0; 

		worker->worker = CreateThread( NULL, 0, worker_thread, worker, 0, NULL ); 
		if( NULL == worker->worker )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI stop_job_worker( PJOB_WORKER worker, ULONG wait_time )
{
	LRESULT ret = ERROR_SUCCESS; 
	INT32 _ret; 
	ULONG wait_ret; 
	PLIST_ENTRY entry; 
	WORKER_JOB *job; 

	do 
	{
		ASSERT( NULL != worker ); 
		ASSERT( FALSE ); 

		worker->stop_working = TRUE; 

		_ret = SetEvent( worker->work_notify ); 
		if( FALSE == _ret )
		{
			SAFE_SET_ERROR_CODE( ret ); 
			//break; 
		}

		wait_ret = WaitForSingleObject( worker->worker, wait_time ); 
		if( wait_ret != WAIT_OBJECT_0 
			&& wait_ret != WAIT_ABANDONED_0 )
		{
			ASSERT( wait_ret != WAIT_FAILED ); 
			TerminateThread( worker->worker, 0x000000ed ); 
		}

		ret = lock_mutex( worker->work_lock ); 
		if( ret != ERROR_SUCCESS )
		{
		}

		for( ; ; )
		{
			entry = worker->works.Flink; 
			if( entry == &worker->works )
			{
				break; 
			}

			RemoveEntryList( entry ); 

			job = CONTAINING_RECORD( entry, WORKER_JOB, entry ); 
			free( job ); 
		}
		
		unlock_mutex( worker->work_lock ); 
		
		CloseHandle( worker->work_lock ); 
		CloseHandle( worker->work_notify ); 
		CloseHandle( worker->worker ); 

		worker->work_lock = NULL; 
		worker->worker = NULL; 
		worker->work_notify = NULL; 

	}while( FALSE );

	return ret; 
}

LRESULT WINAPI add_job_to_worker( PJOB_WORKER worker, PVOID param, LPTHREAD_START_ROUTINE work_routine )
{
	LRESULT ret = ERROR_SUCCESS;
	WORKER_JOB *job = NULL ; 

	do 
	{
		//ASSERT( NULL != param ); 
		ASSERT( NULL != work_routine ); 

		job = ( WORKER_JOB* )malloc( sizeof( WORKER_JOB ) ); 
		if( NULL == job )
		{
			break; 
		}

		job->param = param; 
		job->work_routine = work_routine; 

		ret = lock_mutex( worker->work_lock ); 
		if( ERROR_SUCCESS != ret )
		{
			//break; 
		}

		InsertTailList( &worker->works, 
			&job->entry ); 

		unlock_mutex( worker->work_lock ); 

		job = NULL; 

		SetEvent( worker->work_notify ); 
	}while( FALSE );

	if( NULL != job )
	{
		free( job ); 
	}

	return ret; 
}

LRESULT WINAPI check_all_work_done( PJOB_WORKER worker )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 

	do 
	{
		_ret = check_woker_job_exist( worker ); 
		if( _ret == ERROR_SUCCESS )
		{
			ret = ERROR_NOT_EMPTY; 
			break; 
		}

		if( worker->working_state != WAIT_WORK_JOB )
		{
			ret = ERROR_NOT_EMPTY; 
			break; 
		}
	}while( FALSE );

	return ret; 
}

LRESULT WINAPI check_job_worker_exit( PJOB_WORKER worker )
{
	LRESULT ret = ERROR_SUCCESS; 
	LRESULT _ret; 
	ULONG wait_ret; 
	WORKER_JOB *job; 
	LIST_ENTRY *entry; 

	do 
	{
		if( NULL == worker->worker )
		{
			//ret = ERROR_INVALID_PARAMETER; 
			ASSERT( worker->work_lock == NULL ); 
			ASSERT( worker->work_notify == NULL ); 
			ASSERT( FALSE ); 
			break; 
		}

		wait_ret = WaitForSingleObject( worker->worker, 0 ); 

		if( wait_ret != WAIT_OBJECT_0 
			&& wait_ret != WAIT_ABANDONED_0 
			&& wait_ret != WAIT_FAILED )
		{
			ret = ERROR_ERRORS_ENCOUNTERED; 
			break; 
		}

		ASSERT( wait_ret != WAIT_FAILED ); 

		if( TRUE != worker->stop_working )
		{
			//ASSERT( FALSE ); 
			dbg_print( MSG_FATAL_ERROR, "the woker thread exit, but have not set its stop flag\n" ); 
			//ret = ERROR_INVALID_PARAMETER; 
			//break; 
		}

		ASSERT( NULL != worker->work_notify ); 

		//ASSERT( FALSE ); 
		_ret = lock_mutex( worker->work_lock ); 
		if( _ret != ERROR_SUCCESS )
		{
		}

		if( FALSE == IsListEmpty( &worker->works ) )
		{
			dbg_print( MSG_FATAL_ERROR, "worker exit end job still exist\n" ); 
		}

		for( ; ; )
		{
			entry = worker->works.Flink; 
			if( entry == &worker->works )
			{
				break; 
			}

			RemoveEntryList( entry ); 

			job = CONTAINING_RECORD( entry, WORKER_JOB, entry ); 
			free( job ); 
		}

		unlock_mutex( worker->work_lock ); 

		CloseHandle( worker->work_lock ); 
		CloseHandle( worker->work_notify ); 
		CloseHandle( worker->worker ); 

		worker->work_lock = NULL; 
		worker->worker = NULL; 
		worker->work_notify = NULL; 
	} while ( FALSE ); 

	return ret; 
}