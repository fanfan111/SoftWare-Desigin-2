#include "StdAfx.h"
#include "TaskMgr.h"
#include <process.h>
#include <cassert>

namespace DuiVision
{

BOOL CTaskMgr::Startup(int priority /* = THREAD_PRIORITY_NORMAL */)
{
	if(IsStarted())
		return FALSE;

	m_hThread = (HANDLE)_beginthreadex(NULL, 0, TaskThread, this, CREATE_SUSPENDED, NULL);
	if(m_hThread != (HANDLE)-1)
	{
		::SetThreadPriority(m_hThread, priority);
		::ResumeThread(m_hThread);

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

unsigned CTaskMgr::TaskThread(LPVOID lpVoid)
{
	CTaskMgr *pTM = (CTaskMgr*)lpVoid;

	return pTM->Run();
}

unsigned CTaskMgr::Run()
{
	while(true)
	{
		//
		// 1.�ȴ��������
		//
		::WaitForSingleObject(m_taskEvent, INFINITE);

		// �˳�
		if(IsExited()) break;

		IBaseTask *pTask;
		while(!IsExited() && ((pTask = GetTask()) != NULL))
		{
			//
			// 2.��������
			//
			if(pTask->IsUITask())
			{
				// ��ҪUI�̴߳��������,תUI�߳�
				DuiSystem::RunUITask(pTask, this);
			}else
			{
				// ֱ��ִ��
				if(pTask->TaskProcess(this))
				{
					pTask->TaskNotify(this, IBaseTask::TE_Completed);
				}else
				{
					pTask->TaskNotify(this, IBaseTask::TE_Canceled);
				}
			}
			
			pTask->Release();	// ִ����,���������������ü���,�����0���Զ�ɾ��
		}		
	}

	return 0;
}

BOOL CTaskMgr::Shutdown(DWORD millisec /* = INFINITE */)
{
	if(!IsStarted())
		return FALSE;

	//
	// 1.�˳�
	//
	m_exited = TRUE;
	StartTask();

	//
	// 2.�ȴ�ͣ��
	//
	DWORD ret = WaitForSingleObject(m_hThread, millisec);
	if(ret == WAIT_TIMEOUT)
	{
		TerminateThread(m_hThread, 0);
		WaitForSingleObject(m_hThread, INFINITE);
	}

	CloseHandle(m_hThread);

	//
	// 3.����������
	//
	ClearTaskQueue();

	Init();
	return TRUE;
}

LONG CTaskMgr::NaiveAddTask(IBaseTask *pTask)
{
	if(pTask == NULL)
	{
		assert(0);
		return 0L;
	}

	pTask->AddRef();	// ���������������ü���

	pTask->SetId(m_taskId++);

	NaiveInsertTask(pTask);

	return m_taskId-1;
}

void CTaskMgr::NaiveInsertTask(IBaseTask *pTask)
{
	m_taskQueue.push_back(pTask);

	//
	// ��β��ͷ����������
	//
	TaskIter it = --m_taskQueue.end();
	TaskIter ed = m_taskQueue.begin();
	for(; it != ed; --it)
	{
		TaskIter next = it; --next;
		if((*it)->GetPriority() < (*next)->GetPriority())
		{
			IBaseTask *pTask = *it;
			*it = *next;
			*next = pTask;
		}
		else
		{
			break;
		}
	}
}

IBaseTask* CTaskMgr::GetTask()
{
	IBaseTask *pTask;

	LockTaskQueue();

	if(m_taskQueue.empty())
	{
		pTask = NULL;
	}
	else
	{
		pTask = m_taskQueue.front();
		m_taskQueue.pop_front();
	}

	UnlockTaskQueue();

	return pTask;
}

void CTaskMgr::NaiveClearTaskQueue(TaskQueue &taskQueue)
{
	TaskIter it = taskQueue.begin();
	TaskIter ed = taskQueue.end();
	for(; it != ed; ++it)
	{
		(*it)->TaskNotify(this, IBaseTask::TE_Removed);
		(*it)->Release();
	}
}

void CTaskMgr::ClearTaskQueue()
{
	TaskQueue taskQueue;

	LockTaskQueue();
	{
		taskQueue.swap(m_taskQueue);
	}
	UnlockTaskQueue();
	
	NaiveClearTaskQueue(taskQueue);
}

void CTaskMgr::ClearKindTask(ULONG type)
{
	TaskQueue taskQueue;

	LockTaskQueue();
	{
		TaskIter it = m_taskQueue.begin();
		TaskIter ed = m_taskQueue.end();
		for(; it != ed; )
		{
			TaskIter cur = it++;

			if((*cur)->GetType() == type)
			{
				taskQueue.push_back(*cur);
				m_taskQueue.erase(cur);
			}
		}
	}
	UnlockTaskQueue();

	NaiveClearTaskQueue(taskQueue);
}

void CTaskMgr::NaiveClearKindTask(ULONG type)
{
	TaskQueue taskQueue;

	TaskIter it = m_taskQueue.begin();
	TaskIter ed = m_taskQueue.end();
	for(; it != ed; )
	{
		TaskIter cur = it++;

		if((*cur)->GetType() == type)
		{
			taskQueue.push_back(*cur);
			m_taskQueue.erase(cur);
		}
	}

	NaiveClearTaskQueue(taskQueue);
}

};