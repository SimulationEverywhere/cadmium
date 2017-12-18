#ifndef CADMIUM_COMMON_LOGGERS_HELPERS_HPP
#define CADMIUM_COMMON_LOGGERS_HELPERS_HPP

#include <tuple>
#include <boost/type_index.hpp>
#include <iostream>
#include <cadmium/modeling/message_bag.hpp>

namespace cadmium {
    namespace logger {

        // Displaying all messages in a bag
        //printing all messages in bags, if the support the << operator to ostream
        template <typename T>
        struct is_streamable {
        private:
            template <typename U>
            static decltype(std::cout << std::declval<U>(), void(), std::true_type()) test(int);
            template <typename>
            static std::false_type test(...);
        public:
            using type=decltype(test<T>(0));
            static constexpr auto value=type::value;
        };

        template<typename T, typename V=typename is_streamable<T>::type>
        struct value_or_name;

        template<typename T>
        struct value_or_name<T, std::true_type>{
            static void print(std::ostream& os, const T& v){
                os << v;
            }
        };

        template<typename T>
        struct value_or_name<T, std::false_type>{
            static void print(std::ostream& os, const T& v){
                os << "obscure message of type ";
                os << boost::typeindex::type_id<T>().pretty_name();
            }
        };

        template<typename T>
        std::ostream& implode(std::ostream& os, const T& collection){
            os << "{";
            auto it = std::begin(collection);
            if (it != std::end(collection)) {
                value_or_name<typename T::value_type>::print(os, *it);
                ++it;
            }
            while (it != std::end(collection)){
                os << ", ";
                value_or_name<typename T::value_type>::print(os, *it);
                ++it;
            }
            os << "}";
            return os;
        }

        //priting messages for all ports
        template<size_t s, typename... T>
        struct print_messages_by_port_impl{
            using current_bag=typename std::tuple_element<s-1, std::tuple<T...>>::type;
            static void run(std::ostream& os, const std::tuple<T...>& b){
                print_messages_by_port_impl<s-1, T...>::run(os, b);
                os << ", ";
                os << boost::typeindex::type_id<typename current_bag::port>().pretty_name();
                os << ": ";
                implode(os, cadmium::get_messages<typename current_bag::port>(b));
            }
        };

        template<typename... T>
        struct print_messages_by_port_impl<1, T...>{
            using current_bag=typename std::tuple_element<0, std::tuple<T...>>::type;
            static void run(std::ostream& os, const std::tuple<T...>& b){
                os << boost::typeindex::type_id<typename current_bag::port>().pretty_name();
                os << ": ";
                implode(os, cadmium::get_messages<typename current_bag::port>(b));
            }
        };

        template<typename... T>
        struct print_messages_by_port_impl<0, T...>{
            static void run(std::ostream& os, const std::tuple<T...>& b){}
        };

        template <typename... T>
        void print_messages_by_port(std::ostream& os, const std::tuple<T...>& b){
            os << "[";
            print_messages_by_port_impl<sizeof...(T), T...>::run(os, b);
            os << "]";
        }
    }
}
#endif //CADMIUM_COMMON_LOGGERS_HELPERS_HPP
