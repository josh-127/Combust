#ifndef COMBUST_COMMON_HH
#define COMBUST_COMMON_HH
#include <memory>

#define IN
#define OUT
#define IN_OUT
#define THIS

template<typename Ty>
using Rc = std::shared_ptr<Ty>;

template<typename Ty>
using Owner = std::unique_ptr<Ty>;

template<typename Ty, typename... Types>
[[nodiscard]] auto NewObj(Types&& ... args) -> decltype(std::make_shared<Ty>(args...)) {
    return std::make_shared<Ty>(args...);
}

template<typename Ty, typename... Types>
[[nodiscard]] auto NewChild(Types&& ... args) -> decltype(std::make_unique<Ty>(args...)) {
    return std::make_unique<Ty>(args...);
}

template<typename To, typename From>
[[nodiscard]] auto As(const Rc<From>& obj) -> decltype(std::static_pointer_cast<To>(obj)) {
    return std::static_pointer_cast<To>(obj);
}

class Object {
public:
    explicit Object() {}
private:
    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;
};

#endif
