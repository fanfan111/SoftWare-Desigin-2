#ifndef __DUIVISION_TASK_MGR_H_
#define __DUIVISION_TASK_MGR_H_

#include <list>
#include "BaseTask.h"

namespace DuiVision
{

//
// �������
//
class CTaskMgr
{
public:
	typedef std::list<IBaseTask*>		TaskQueue;
	typedef TaskQueue::iterator			TaskIter;
	typedef TaskQueue::const_iterator	TaskCIter;
	
public:
	CTaskMgr();
	~CTaskMgr();

	//
	// ��ָ�����߳����ȼ�������������
	//
	BOOL Startup(int priority = THREAD_PRIORITY_NORMAL);
	BOOL IsStarted() const;
	BOOL IsExited() const;
	BOOL Shutdown(DWORD millisec = INFINITE);

	//
	// ��ʼִ������
	//
	BOOL StartTask();

	//
	// ����/�����������
	//
	//@Remark
	// ���е�Naive��ͷ�ĺ�������Ҫ�������������
	//
	void TryLockQueue();
	void LockTaskQueue();
	void UnlockTaskQueue();

	void ClearTaskQueue();
	void ClearKindTask(ULONG type);
	void NaiveClearKindTask(ULONG type);

	BOOL IsEmpty();
	ULONG GetTaskCnt();

	// ����������У�ֱ�Ӳ���(����)
	TaskQueue& NaiveGetTaskQueue();

	//
	// ����ǰ�������ȼ�˳����뵽���������
	//
	//@Remark
	// ��Ҫ����StartTask����������
	//
	LONG AddTask(IBaseTask *pTask);
	LONG NaiveAddTask(IBaseTask *pTask);
	void NaiveInsertTask(IBaseTask *pTask);

private:
	void Init();
	IBaseTask* GetTask();
	void NaiveClearTaskQueue(TaskQueue &taskQueue);

	unsigned Run();
	static unsigned __stdcall TaskThread(LPVOID lpVoid);

private:
	BOOL				m_exited;
	LONG				m_taskId;		// �������Id
	HANDLE				m_hThread;		// �����߳̾��
	HANDLE				m_taskEvent;	// ��������֪ͨ�¼�
	TaskQueue			m_taskQueue;	// �������
	CRITICAL_SECTION	m_taskLock;		// ���������
};
//////////////////////////////////////////////////////////////////////////
inline CTaskMgr::CTaskMgr() 
{ 
	Init(); 
	::InitializeCriticalSection(&m_taskLock);
	m_taskEvent = ::CreateEventW(NULL, FALSE, FALSE, NULL);
}
inline CTaskMgr::~CTaskMgr()
{ 
	Shutdown(); 
	::DeleteCriticalSection(&m_taskLock);
	::CloseHandle(m_taskEvent);
}

inline BOOL CTaskMgr::IsStarted() const 
{ return (m_hThread != (HANDLE)-1); }

inline BOOL CTaskMgr::StartTask()
{
	if(!IsStarted())
		return FALSE;

	::SetEvent(m_taskEvent);
	return TRUE;
}

inline void CTaskMgr::TryLockQueue() 
{ ::TryEnterCriticalSection(&m_taskLock); }
inline void CTaskMgr::LockTaskQueue() 
{ ::EnterCriticalSection(&m_taskLock); }
inline void CTaskMgr::UnlockTaskQueue() 
{ ::LeaveCriticalSection(&m_taskLock); }

inline CTaskMgr::TaskQueue& CTaskMgr::NaiveGetTaskQueue() 
{ return m_taskQueue; }

inline void CTaskMgr::Init()
{
	m_exited = FALSE;
	m_taskId = 1;
	m_hThread = (HANDLE)-1;
}

inline LONG CTaskMgr::AddTask(IBaseTask *pTask)
{	
	ULONG id;
	LockTaskQueue();
	id = NaiveAddTask(pTask);
	UnlockTaskQueue();
	return id;
}

inline ULONG CTaskMgr::GetTaskCnt()
{
	ULONG cnt;
	LockTaskQueue();
	cnt = static_cast<ULONG>(m_taskQueue.size());
	UnlockTaskQueue();
	return cnt;
}

inline BOOL CTaskMgr::IsEmpty()
{ return (GetTaskCnt() == 0);}

inline BOOL CTaskMgr::IsExited() const 
{ return m_exited; }

};

#endif	// __DUIVISION_TASK_MGR_H_