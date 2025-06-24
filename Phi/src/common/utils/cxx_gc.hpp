/*
 * @File	  : cxx_gc.hpp
 * @Coding	  : utf-8
 * @Author    : qq285503851
 * @Time      : 2022/11/27 15:32
 * @Introduce : C++ GC
*/

#include <set>
#include <map>

#ifndef defined(GC_HPP)
#define GC_HPP  

// For thread safety  为了线程安全
//#define THREAD_SAFETY 1

#if THREAD_SAFETY
#include <mutex>
#define MUTEXT_LOCK  mMutex.lock()  
#define MUTEXT_UNLOCK  mMutex.unlock()  
#else
#define MUTEXT_LOCK  
#define MUTEXT_UNLOCK  
#endif

// Base class for all objects that are tracked by
// the garbage collector.
//垃圾收集器跟踪的所有对象的基类。
class GCObject
{
public:

	// For mark and sweep algorithm. When a GC occurs
	// all live objects are traversed and mMarked is
	// set to true. This is followed by the sweep phase
	// where all unmarked objects are deleted.
	// 用于标记和扫描算法。当GC发生时，将遍历所有活动对象，
	//并将mMarked设置为true。随后是扫描阶段，删除所有未标记的对象。
	bool mMarked;

public:
	GCObject();
	GCObject(GCObject const&);
	virtual ~GCObject();

	// Mark the object and all its children as live
	// 将对象及其所有子对象标记为活动
	void mark();

	// Overridden by derived classes to call mark()
	// on objects referenced by this object. The default
	// implemention does nothing.
	// 被派生类重写，以对该对象引用的对象调用mark（）。默认实现不起作用。
	virtual void markChildren();

public:
	void* operator new(size_t size);
	void* operator new(size_t size, void* p);
	void operator delete(void* p);

protected:
	void* operator new[](size_t size);
	void operator delete[](void* p);
};

// Wrapper for an array of bytes managed by the garbage
// collector.
// 垃圾收集器管理的字节数组的包装器。
class GCMemory : public GCObject
{
public:
	unsigned char* mMemory;
	int   mSize;

public:
	GCMemory(int size);
	virtual ~GCMemory();

	unsigned char* get();
	int size();
};

// Garbage Collector. Implements mark and sweep GC algorithm.
// 垃圾收集器。实现标记和扫描GC算法。
class GC
{
public:
	// Global garbage collector object
	// 全局垃圾收集器对象
	static GC instance;

public:
	// A collection of all active heap objects.
	// 所有活动堆对象的集合。
	typedef std::set<GCObject*> ObjectSet;
	ObjectSet mHeap;

	// Collection of objects that are scanned for garbage.
	// 扫描垃圾的对象集合。
	ObjectSet mRoots;

	// Pinned objects
	// 固定的对象
	typedef std::map<GCObject*, unsigned int> PinnedSet;
	PinnedSet mPinned;

public:
	// Perform garbage collection. 
	// 执行垃圾收集。
	void collect();

	// Add a root object to the collector.
	// 将根对象添加到收集器。
	void addRoot(GCObject* root);
	// Remove a root object from the collector.
	// 从收集器中删除根对象。
	void removeRoot(GCObject* root);

	// Pin an object so it temporarily won't be collected. 
	// Pinned objects are reference counted. Pinning it
	// increments the count. Unpinning it decrements it. When
	// the count is zero then the object can be collected.
	// 固定对象，使其暂时不会被收集。
	// 固定对象被引用计数。锁定它
	// 增加计数。松开它会使它变小。当计数为零时，则可以收集对象。
	void pin(GCObject* o);
	void unpin(GCObject* o);

	// Add an heap allocated object to the collector.
	// 将堆分配的对象添加到收集器。
	void addObject(GCObject* o);
	// Remove a heap allocated object from the collector.
	// 从收集器中删除堆分配的对象。
	void removeObject(GCObject* o);

	// Go through all objects in the heap, unmarking the live
	// objects and destroying the unreferenced ones.
	// 遍历堆中的所有对象，取消标记活动对象并销毁未引用的对象。
	void sweep();

	// Number of live objects in heap
	// 堆中的活动对象数
	int liveCount();

protected:

#if THREAD_SAFETY
	std::mutex mMutex;
#endif

};

#endif

#include <vector>

// GCObject
GCObject::GCObject()
	: mMarked(false)
{

}

GCObject::GCObject(GCObject const&)
	: mMarked(false)
{

}

GCObject::~GCObject()
{
}

void GCObject::mark()
{
	if (!mMarked)
	{
		mMarked = true;
		markChildren();
	}
}

void GCObject::markChildren()
{
}

void* GCObject::operator new(size_t size)
{
	void* p = ::operator new(size);
	GC::instance.addObject((GCObject*)p);
	return p;
}

void* GCObject::operator new(size_t size, void* p)
{
	return ::operator new(size, p);
}

void GCObject::operator delete(void* p)
{
	GC::instance.removeObject((GCObject*)p);
	::operator delete(p);
}

void* GCObject::operator new[](size_t size)
{
	void* p = ::operator new[](size);
	return p;
}

void GCObject::operator delete[](void* p)
{
	::operator delete[](p);
}

// GCMemory
GCMemory::GCMemory(int size)
	: mSize(size)
{
	mMemory = new unsigned char[size];
}

GCMemory::~GCMemory()
{
	delete[] mMemory;
}

unsigned char* GCMemory::get()
{
	return mMemory;
}

int GCMemory::size()
{
	return mSize;
}


// GarbageCollector
GC GC::instance;

void GC::collect()
{
	MUTEXT_LOCK;
	// Mark root objects
	for (ObjectSet::iterator it = mRoots.begin();
		it != mRoots.end(); ++it)
	{
		(*it)->mark();
	}

	// Mark pinned objects
	for (PinnedSet::iterator it = mPinned.begin();
		it != mPinned.end(); ++it)
	{
		(*it).first->mark();
	}
	MUTEXT_UNLOCK;

	sweep();
}

void GC::addRoot(GCObject* root)
{
	MUTEXT_LOCK;
	mRoots.insert(root);
	MUTEXT_UNLOCK;
}

void GC::removeRoot(GCObject* root)
{
	MUTEXT_LOCK;
	mRoots.erase(root);
	MUTEXT_UNLOCK;
}

void GC::pin(GCObject* o)
{
	MUTEXT_LOCK;
	PinnedSet::iterator it = mPinned.find(o);
	if (it == mPinned.end())
	{
		mPinned.insert(std::make_pair(o, 1));
	}
	else
	{
		(*it).second++;
	}
	MUTEXT_UNLOCK;
}

void GC::unpin(GCObject* o)
{
	MUTEXT_LOCK;
	do
	{
		PinnedSet::iterator it = mPinned.find(o);
		if (it == mPinned.end())
			break;

		if (--((*it).second) == 0)
			mPinned.erase(it);
	} while (0);
	MUTEXT_UNLOCK;
}

void GC::addObject(GCObject* o)
{
	MUTEXT_LOCK;
	mHeap.insert(o);
	MUTEXT_UNLOCK;
}

void GC::removeObject(GCObject* o)
{
	MUTEXT_LOCK;
	mHeap.erase(o);
	MUTEXT_UNLOCK;
}

void GC::sweep()
{
	std::vector<GCObject*> erase;

	MUTEXT_LOCK;
	for (ObjectSet::iterator it = mHeap.begin();
		it != mHeap.end(); ++it)
	{
		GCObject* p = *it;
		if (p->mMarked)
		{
			p->mMarked = false;
		}
		else
		{
			erase.push_back(*it);
		}
	}
	MUTEXT_UNLOCK;

	for (std::vector<GCObject*>::iterator it = erase.begin();
		it != erase.end(); ++it)
	{
		delete* it;
	}
}

int GC::liveCount()
{
	MUTEXT_LOCK;
	int nCount = mHeap.size();
	MUTEXT_UNLOCK;
	return nCount;
}