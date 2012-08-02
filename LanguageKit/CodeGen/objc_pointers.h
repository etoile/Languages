#import <Foundation/NSObject.h>
#undef _
#include <tr1/unordered_map>
namespace {
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
template <typename K, typename V> struct object_map : public std::tr1::unordered_map<K, V, object_hash<K>, object_equal<K> > {};
}
