// https://raw.githubusercontent.com/aminya/move_const/0.1.0/move_const.h

#include <type_traits>

template <typename T>
typename std::remove_const<typename std::remove_reference<T>::type>::type&& move_const(T&& arg) noexcept
{
    return const_cast<typename std::remove_const<typename std::remove_reference<T>::type>::type&&>(
        static_cast<typename std::remove_reference<T>::type&&>(arg)
    );
}
