#import <Foundation/NSObject.h>
#undef _
#include <tr1/unordered_map>
#if !__has_feature(objc_arc)

#ifdef  __GNUSTEP_RUNTIME__
#	import <objc/capabilities.h>
#endif
#ifdef OBJC_CAP_ARC
extern "C"{
#import <objc/objc-arc.h>
}
#else
static inline id objc_retain(id x) { return [x retain]; }
static inline void objc_release(id x) { [x release]; }
static inline id objc_loadWeak(id* object) { return *object; }
static inline id objc_initWeak(id *object, id value)  { *object = value; }
static inline void objc_destroyWeak(id* addr) {}
static inline void objc_copyWeak(id *dest, id *src) { *dest = *src; }
#warning 
#endif
namespace {
template<typename X>
struct strong_id
{
	X o;
	strong_id() : o(nil) {}
	strong_id(strong_id<X> const &x) : o(objc_retain(x.o)) {}
	strong_id(X x) : o(objc_retain(x)) {}
	~strong_id() { objc_release(o); }
	inline operator id() const { return o; }
	inline operator id() { return o; }
	inline bool operator ==(const strong_id &other) const
	{
		return o == other.o;
	}
	inline bool operator ==(const id &other) const
	{
		return o == other;
	}
};
template<typename X>
struct weak_id
{
	X o;
	weak_id() : o(nil) {}
	weak_id(strong_id<X> const &x)
	{
		objc_initWeak(&o, x.o);
	}
	weak_id(X x)
	{
		objc_initWeak(&o, x);
	}
	weak_id(weak_id<X> const &x)
	{
		objc_copyWeak(&o, (X*)&x.o);
	}
	~weak_id() { objc_destroyWeak(&o); }
	inline operator id() const { return objc_loadWeak((X*)&o); }
	inline operator id() { return objc_loadWeak((X*)&o); }
	inline bool operator ==(const weak_id &other) const
	{
		return o == other.o;
	}
	inline bool operator ==(const id &other) const
	{
		return o == other;
	}
};
#else
typedef id strong_id;
typedef __weak id weak_id;
#endif
template <typename X>
struct object_equal
{
	bool operator()(const X s1, const X s2) const
	{
		return (s1 == s2) || [(id)s1 isEqual: (id)s2];
	}
};

template <typename X>
struct object_hash
{
	size_t operator()(const X s1) const
	{
		return (size_t)[(id)s1 hash];
	}
};
template <typename K, typename V> struct object_map : public std::tr1::unordered_map<strong_id<K>, V, object_hash<K>, object_equal<K> > {};
}
