#ifndef PTC_CL_COMMON_H
#define PTC_CL_COMMON_H
#include <memory>

#define IN
#define OUT
#define IN_OUT
#define THIS

template<typename Ty>
using Rc = std::shared_ptr<Ty>;

template<typename Ty, typename... Types>
[[nodiscard]] auto NewObj(Types&& ... args) -> decltype(std::make_shared<Ty>(args...)) {
    return std::make_shared<Ty>(args...);
}

#endif
