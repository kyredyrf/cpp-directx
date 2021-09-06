#pragma once

#include <memory>
#include <source_location>

//template <class _Ty, class... _Types>
//inline _Ty* new_normal(_Types&&... _Args, const std::source_location& location = std::source_location::current());
template <class _Ty>
inline _Ty* new_normal(const std::source_location& location = std::source_location::current());
template <class _Ty, class _Ty1>
inline _Ty* new_normal(_Ty1&& p1, const std::source_location& location = std::source_location::current());
template <class _Ty, class _Ty1, class _Ty2>
inline _Ty* new_normal(_Ty1&& p1, _Ty2&& p2, const std::source_location& location = std::source_location::current());

template <class _Ty>
inline void delete_normal(_Ty* ptr, const std::source_location& location = std::source_location::current());

/**
 *  new_normal 用のデリーター
 */
template <class _Ty>
struct normal_deleter
{
    constexpr normal_deleter() noexcept = default;

    template <class _Ty2, std::enable_if_t<std::is_convertible_v<_Ty2*, _Ty*>, int> = 0>
    normal_deleter(const normal_deleter<_Ty2>&) noexcept {}

    void operator()(_Ty* _Ptr) const noexcept /* strengthened */
    {
        static_assert(0 < sizeof(_Ty), "can't delete an incomplete type");
        delete_normal(_Ptr);
    }
};

/**
 *  normal_deleter をデリーターとして使用する unique_ptr
 */
template <class _Ty>
using normal_unique_ptr = std::unique_ptr<_Ty, normal_deleter<_Ty>>;

/**
 *  new_normal 用の make_unique
 */
template <class _Ty, class... _Types, std::enable_if_t<!std::is_array_v<_Ty>, int> = 0>
_NODISCARD normal_unique_ptr<_Ty> make_unique(_Types&&... _Args)
{
    return normal_unique_ptr<_Ty>(new_normal<_Ty>(_STD forward<_Types>(_Args)...));
}



/**
 *  Visual C++ 向け実装になっているので注意
 */
#ifdef WIN32

// Memory block identification
#define _FREE_BLOCK      0
#define _NORMAL_BLOCK    1
#define _CRT_BLOCK       2
#define _IGNORE_BLOCK    3
#define _CLIENT_BLOCK    4
#define _MAX_BLOCKS      5

//template <class _Ty, class... _Types>
//inline _Ty* new_normal(_Types&&... _Args, const std::source_location& location)
//{
//    auto ptr = static_cast<_Ty*>(::operator new(sizeof(_Ty), _NORMAL_BLOCK, location.file_name(), location.line()));
//    new (ptr) _Ty(_STD forward<_Types>(_Args)...);
//    return ptr;
//}
template <class _Ty>
inline _Ty* new_normal(const std::source_location& location)
{
    auto ptr = static_cast<_Ty*>(::operator new(sizeof(_Ty), _NORMAL_BLOCK, location.file_name(), location.line()));
    new (ptr) _Ty();
    return ptr;
}
template <class _Ty, class _Ty1>
inline _Ty* new_normal(_Ty1&& p1, const std::source_location& location)
{
    auto ptr = static_cast<_Ty*>(::operator new(sizeof(_Ty), _NORMAL_BLOCK, location.file_name(), location.line()));
    new (ptr) _Ty(p1);
    return ptr;
}
template <class _Ty, class _Ty1, class _Ty2>
inline _Ty* new_normal(_Ty1&& p1, _Ty2&& p2, const std::source_location& location)
{
    auto ptr = static_cast<_Ty*>(::operator new(sizeof(_Ty), _NORMAL_BLOCK, location.file_name(), location.line()));
    new (ptr) _Ty(p1, p2);
    return ptr;
}

template <class _Ty>
inline void delete_normal(_Ty* ptr, const std::source_location& location)
{
    ptr->~_Ty();
    ::operator delete(ptr, _NORMAL_BLOCK, location.file_name(), location.line());
}

#undef _FREE_BLOCK
#undef _NORMAL_BLOCK
#undef _CRT_BLOCK
#undef _IGNORE_BLOCK
#undef _CLIENT_BLOCK
#undef _MAX_BLOCKS

#endif // WIN32
