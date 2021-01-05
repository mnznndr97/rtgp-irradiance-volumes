#pragma once

#include <Platform.hpp>
#include <cassert>
#include <functional>

#ifdef ISWINPLATFORM
#include <ppl.h>
#include <concurrent_queue.h>
#include <concurrent_unordered_map.h>
#else
// Linux seems to not have built-in concurret collections. To simply things, we just use a mutex even
// if is not the best for performance
#include <thread>
#include <mutex>
#include <queue>
#include <unordered_map>
#endif

/// <summary>
/// Very basic and simple array pool mechanism implementation
/// 
/// <p>
/// This array pool itsel may not be the best in terms of perfomance (a pass in two concurrent collection is required)
/// but for our purposes it should not be too limiting
/// </p>
/// </summary>
/// <remarks>
/// For our purposes the array pool provides a callback to initialize a newly created arary
/// which can be very useful if a single pool instance is used in the same multithreaded scenario
/// 
/// <p>
/// Unlike the C# ArrayPool, here to avoid pointer leak and to simplify things, the ArrayPool does not provide
/// a static interface.
/// </p>
/// </remars>
template <class T>
class SimpleArrayPool
{
private:
#ifdef ISWINPLATFORM
	concurrency::concurrent_queue<T*> _availablePoolItems;
	concurrency::concurrent_unordered_map<long, int> _pointerSizesMap;
#else
	std::mutex _collectionsMutex;
	queue<T*> _availablePoolItems;
	unordered_map<long, int> _pointerSizesMap;
#endif

	/// <summary>
	/// Creates and initialize in a user-defined way an array that will be part of the pool
	/// </summary>
	T* GetNewArray(int minimumSize, const std::function<void(T*)>& initializer) {
		T* result = new T[minimumSize];
		initializer(result);

		long arraryAddress = reinterpret_cast<long>(result);
		// We have to store the array size to be able to check in the future
		// if the requested lentgh is the same of the pooled array
		_pointerSizesMap[arraryAddress] = minimumSize;
		return result;
	}

public:
	// Let's avoid copies to don't end up with the same pointers in two places
	SimpleArrayPool<T>(const SimpleArrayPool<T>&) = delete;
	void operator=(const SimpleArrayPool<T>&) = delete;

	// Maybe a move semantic can be safely implement but for our purposes is not needed

	SimpleArrayPool();
	~SimpleArrayPool();

	T* Rent(int size, const std::function<void(T*)>& initializer);
	void Return(T* pool);
};

template<class T>
SimpleArrayPool<T>::SimpleArrayPool()
{

}

template<class T>
SimpleArrayPool<T>::~SimpleArrayPool()
{
#ifdef ISWINPLATFORM
	for (auto it = _availablePoolItems.unsafe_begin(); it != _availablePoolItems.unsafe_end(); ++it) {
		delete[](*it);
	}
	_availablePoolItems.clear();
	_pointerSizesMap.clear();
#else	
	while(!_availablePoolItems.empty()){
		T* item = _availablePoolItems.front();
		_availablePoolItems.pop();
		delete[] item; 
	}
	_pointerSizesMap.clear();
#endif	
}

template<class T>
T* SimpleArrayPool<T>::Rent(int size, const std::function<void(T*)>& initializer)
{
#ifdef ISWINPLATFORM	
	T* result = nullptr;
	
	if (!_availablePoolItems.try_pop(result)) {
		return GetNewArray(size, initializer);
		// Nothing else to do. The array will stored 
		// when return is called
	}
	else
	{
		long arraryAddress = reinterpret_cast<long>(result);
		int oldPointerSize = _pointerSizesMap.at(arraryAddress);

		if (size != oldPointerSize) {
			// We have to deallocate old array
			delete[] result;
			return GetNewArray(size, initializer);
		}
		else {
			return result;
		}
	}
	
#else	
	// Let's just use a simple lock for linux
	const std::lock_guard<std::mutex> lock(_collectionsMutex);
	
	if (_availablePoolItems.empty()) {
		return GetNewArray(size, initializer);
		// Nothing else to do. The array will stored 
		// when return is called
	}
	else
	{
		T* result = _availablePoolItems.front();
		_availablePoolItems.pop();
		long arraryAddress = reinterpret_cast<long>(result);
		int oldPointerSize = _pointerSizesMap.at(arraryAddress);

		if (size != oldPointerSize) {
			// We have to deallocate old array
			delete[] result;
			return GetNewArray(size, initializer);
		}
		else {
			return result;
		}
	}
#endif	
}

template<class T>
void SimpleArrayPool<T>::Return(T* pool)
{
#ifdef ISWINPLATFORM	
	_availablePoolItems.push(pool);
#else
	// Let's just use a simple lock for linux
	const std::lock_guard<std::mutex> lock(_collectionsMutex);
	_availablePoolItems.push(pool);
#endif		
}

