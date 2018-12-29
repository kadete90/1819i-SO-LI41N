#include <Windows.h>
#include <winnt.h>


class MyCompletionTask {

private:

	long _refCount;

public:

	MyCompletionTask() {
		_refCount = 0;
	}

public: // Reference counting implementation

   // Note ref-counting mechanism must be thread-safe,
   // so we use the Interlocked functions.

	void AddRef()
	{
		InterlockedIncrement(&_refCount);
	}

	void Release()
	{
		long newCount = InterlockedDecrement(&_refCount);

		if (newCount == 0) {
			DoCompletionTask();
			delete this;
		}
	}

private:

	void DoCompletionTask()
	{
		// TODO: Do your thing here
	}
};