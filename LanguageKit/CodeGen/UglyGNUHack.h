#ifdef __linux__
#include <bits/move.h>
@class NSString;
namespace std _GLIBCXX_VISIBILITY(default)
{
_GLIBCXX_BEGIN_NAMESPACE_VERSION

    template<>
    inline NSString *__strong *
    __addressof(NSString *__strong & __r)
    {
      return &__r;
    }
_GLIBCXX_END_NAMESPACE_VERSION
} // namespace

#endif
