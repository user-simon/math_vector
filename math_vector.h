#pragma once
#include <string>
#include <functional>
#include <tuple>
#include <type_traits>
#include <optional>
#include <cmath>

// defined for explicit return types
#define MV_EXPR auto

// standard vector specialization definitions
#define MV_SPEC_DEFS(n, names)                                       \
    private:                                                         \
        using base = vector_spec<T, n - 1>;                          \
    public:                                                          \
       constexpr static char component_names[] = names;              \
       vector_spec& operator= (const vector_spec&) { return *this; }

// defines a binary vector/expression and assignment operator
#define MV_DEF_BIN_OP(op, strict)                                     \
    template<class L, class R,                                        \
        class = std::enable_if_t<mv_impl::is_valid_op<L, R, strict>>> \
    MV_EXPR operator##op (const L& l, const R& r)                     \
    {                                                                 \
        return mv_impl::expr                                          \
        {                                                             \
          [](const auto& l, const auto& r) { return l op r; },        \
          l, r                                                        \
        };                                                            \
    }                                                                 \
                                                                      \
    template<class L, class R,                                        \
        class = std::enable_if_t<mv_impl::is_valid_op<L, R, true>>>   \
    L& operator##op##=(L& l, const R& r)                              \
    {                                                                 \
        return l = l op r;                                            \
    }

// utility for creating a unary expression from operation and operand
#define MV_UN_EXPR(op, val) mv_impl::expr([](const auto& v) { return op(v); }, val)

// defines a unary vector/expression operator
#define MV_DEF_UN_OP(op)                                                    \
    template<class T, class = std::enable_if_t<mv_impl::is_vec_or_expr<T>>> \
    MV_EXPR operator##op (T& v)                                             \
    {                                                                       \
        return MV_UN_EXPR(op, v);                                           \
    }

// defines a type and size alias for math_vector
#define MV_DEF_TYPE(t, t_name, n) using t_name##n##d = math_vec##n##d<t>

// defines type aliases for math_vector with specified size
#define MV_DEF_TYPES(n)                                         \
    template<class T> using math_vec##n##d = math_vector<T, n>; \
    MV_DEF_TYPE(char,          char,   n);                      \
    MV_DEF_TYPE(unsigned char, uchar,  n);                      \
    MV_DEF_TYPE(int,           int,    n);                      \
    MV_DEF_TYPE(unsigned int,  uint,   n);                      \
    MV_DEF_TYPE(float,         float,  n);                      \
    MV_DEF_TYPE(double,        double, n);                      \

// utility for looping over all elements in vector with index
#define MV_LOOP(i) for (unsigned i = 0; i < N; i++)

/*
 *  arithmetic type aliases
 *      define MV_NO_TYPES to avoid potential collisions
*/

#ifndef MV_NO_TYPES
using uint  = unsigned int;
using ulong = unsigned long;
using uchar = unsigned char;
using byte  = uchar;
#endif

namespace mv_impl
{
    /*
     *  forward declares
    */

    template<class, class...>
    struct expr;

    template<class, unsigned>
    struct vector;

    /*
     *  is_vec_or_expr
     *      true if type is some instantiation of vector or expr
    */

    template<class>
    constexpr bool is_vec_or_expr = false;

    template<class OP, class... ARGS>
    constexpr bool is_vec_or_expr<expr<OP, ARGS...>> = true;

    template<class T, unsigned N>
    constexpr bool is_vec_or_expr<vector<T, N>> = true;

    /*
     *  is_valid_op
     *     true if the two types form a valid vector operation.
     *     STRICT_ORDERING = true requires the vector to appear
     *     first in the operation
    */

    template<class T, class U, bool STRICT_ORDERING>
    constexpr bool is_valid_op = (is_vec_or_expr      <T> && is_vec_or_expr      <U>) ||
                                 (is_vec_or_expr      <T> && std::is_arithmetic_v<U>) ||
                                 (std::is_arithmetic_v<T> && is_vec_or_expr      <U>) && !STRICT_ORDERING;
    
    /*
     *  has_named_components
     *      true if type has member "component_names"
     *      used for vector::to_string
    */

    template<class, class = void>
    constexpr bool has_named_components = false;

    template<class T>
    constexpr bool has_named_components<T, std::void_t<decltype(T::component_names)>> = true;

    /*
     *  expr
     *      class to represent expression at compile time
     *      https://en.wikipedia.org/wiki/Expression_templates
    */

    template<class OP, class... ARGS>
    class expr
    {
    private:
        const OP& op;
        const std::tuple<ARGS...> args;
        
        // if T is array, get value at index i
        // otherwise, return value
        template<class T>
        constexpr static auto _get_operand(const T& v, const unsigned i)
        {
            if constexpr(is_vec_or_expr<T>)
                return v[i];
            else
                return v;
        }

    public:
        expr(const OP& op, const ARGS&... args) : op(op), args(args...) {}

        // evaluates value at index i
        constexpr inline auto operator[](const unsigned i) const
        {
            const auto& apply_op = [this, i](const ARGS&... a)
            {
                return op(_get_operand(a, i)...);
            };
            return std::apply(apply_op, args);
        }
    };

    /*
     *  vector_spec
     *      provides size-specific functionality of vector
    */

    template<class T, unsigned N>
    struct vector_spec
    {
        constexpr vector_spec(T*) {}
    };

    template<class T>
    struct vector_spec<T, 2>
    {
        MV_SPEC_DEFS(2, "xy");

        T& x;
        T& y;

        constexpr vector_spec(T data[2]) : x(data[0]), y(data[1]) {}

        double angle() const
        {
            return std::atan2((double)y, x);
        }

#ifdef SFML
        template<typename U>
        operator sf::Vector2<U>() const
        {
            return sf::Vector2<U>(x, y);
        }
#endif
    };

    template<class T>
    struct vector_spec<T, 3> : private vector_spec<T, 2>
    {
        MV_SPEC_DEFS(3, "xyz");

        using base::x;
        using base::y;
        T& z;

        constexpr vector_spec(T data[3]) : base(data), z(data[2]) {}

        // calculates the cross product with another vector
        template<typename U>
        constexpr vector<T, 3> cross(const vector<U, 3>& v) const
        {
            vector<T, 3> out;

            out.x = y * v.z - z * v.y;
            out.y = z * v.x - x * v.z;
            out.z = x * v.y - y * v.x;

            return out;
        }

#ifdef SFML
        template<typename U>
        operator sf::Vector3<U>() const
        {
            return sf::Vector3<U>(x, y, z);
        }
#endif
    };

    template<class T>
    struct vector_spec<T, 4> : private vector_spec<T, 3>
    {
        MV_SPEC_DEFS(4, "xyzw");

        using base::x;
        using base::y;
        using base::z;
        T& w;

        constexpr vector_spec(T data[4]) : base(data), w(data[3]) {}
    };

    /*
     *  vector class
    */

    template<typename T, unsigned N>
    struct vector : public vector_spec<T, N>
    {
        static_assert(std::is_arithmetic_v<T> && N > 1);
        
        using spec = vector_spec<T, N>;
        using type = T;
        constexpr static unsigned size = N;
        constexpr static bool has_named_components = has_named_components<vector>;
        const static vector zero;

        T data[N];

        /*
         *  ctors
        */
        
        // default ctor - sets all components to 0
        constexpr vector() : data{ 0 }, spec(data) {}
        
        // value ctor - specify a value for each component
        template<class...ARGS, class = std::enable_if_t<sizeof...(ARGS) == N && std::conjunction_v<std::is_arithmetic<ARGS>...>>>
        constexpr vector(ARGS... args) : data{ (T)args... }, spec(data) {}
        
        // copy ctor - copies values from other vector
        inline vector(const vector& v) : spec(data)
        {
            eval(v);
        }

        // evaluation ctor - evaluates or copies values from another expression or vector, respectively
        template<class SRC, class = std::enable_if_t<is_vec_or_expr<SRC>>>
        inline vector(const SRC& src) : spec(data)
        {
            eval(src);
        }

        /*
         *  operators
        */

        // true if length > 0
        constexpr operator bool() const
        {
            MV_LOOP(i)
            {
                if (data[i])
                    return true;
            }
            return false;
        }
        
        // gets value at index i
        T& operator[](const unsigned i)
        {
            return data[i];
        }

        // gets value at index i
        constexpr T operator[](const unsigned i) const
        {
            return data[i];
        }

        // performs component-wise comparison of two vectors
        template<class U>
        constexpr bool operator==(const vector<U, N>& v) const
        {
            MV_LOOP(i)
            {
                if (data[i] != v[i])
                    return false;
            }
            return true;
        }

        // performs component-wise comparison of two vectors
        template<class U>
        constexpr bool operator!=(const vector<U, N>& v) const
        {
            return !(*this == v);
        }

        /*
         *  math functions
        */

        // calculates the length squared
        constexpr T length2() const
        {
            return dot(*this);
        }

        // calculates the length
        // equivalent to std::sqrt(length2())
        double length() const
        {
            return std::sqrt((double)length2());
        }

        // calculates the distance squared
        template<class U>
        constexpr double distance2(const vector<U, N>& v) const
        {
            vector<double, N> delta = component_cast<double>() - v;
            return delta.length2();
        }

        // calculates the distance
        // equivalent to std::sqrt(distance2())
        template<class U>
        double distance(const vector<U, N>& v) const
        {
            return std::sqrt((double)distance2(v));
        }

        // normalizes vector
        // returns *this if length == 0
        vector<double, N> normalize() const
        {
            const double len = length();

            if (len == 0)
                return *this;
            return *this / len;
        }

        // sets length
        // equivalent to normalize() * s
        template<class U, class = std::enable_if_t<std::is_arithmetic_v<U>>>
        vector<double, N> set_length(U s) const
        {
            return normalize() * s;
        }

        // calculates the delta_angle between two vectors
        template<class U>
        double delta_angle(const vector<U, N>& v) const
        {
            return std::acos(dot(v) / std::sqrt((double)length2() * v.length2()));
        }

        // calculates the dot product of two vectors
        template<class U>
        constexpr T dot(const vector<U, N>& v) const
        {
            T out = 0;

            MV_LOOP(i)
            {
                out += data[i] * v[i];
            }
            return out;
        }
        
        // calculates the sum of all components in vector
        constexpr T sum() const
        {
            T out = 0;

            MV_LOOP(i)
            {
                out += data[i];
            }
            return out;
        }

        constexpr T prod() const
        {
            T out = 0;

            MV_LOOP(i)
            {
                out *= data[i];
            }
            return out;
        }

        // calculates the absolute value of all components in vector
        vector abs() const
        {
            return MV_UN_EXPR(std::abs, *this);
        }

        // rounds components in vector to nearest whole number
        vector round() const
        {
            return MV_UN_EXPR(std::round, *this);
        }

        // rounds components in vector to nearest whole number
        vector floor() const
        {
            return MV_UN_EXPR(std::floor, *this);
        }

        // rounds components in vector to nearest whole number
        vector ceil() const
        {
            return MV_UN_EXPR(std::ceil, *this);
        }

        /*
         *  utility
        */

        // evaluates or copies values from another expression or vector, respectively
        template<class SRC, class = std::enable_if_t<is_vec_or_expr<SRC>>>
        inline void eval(const SRC& src)
        {
            MV_LOOP(i)
            {
                data[i] = src[i];
            }
        }

        // casts all components to specified type
        template<class U, class = std::enable_if_t<std::is_arithmetic_v<U>>>
        MV_EXPR component_cast() const
        {
            return MV_UN_EXPR(U, *this);
        }

        // serializes vector to string
        std::string to_string(std::optional<std::string> name = {}, bool line_breaks = false) const
        {
            std::string out;

            if (name)
                out = *name + (line_breaks ? "\n" : "  ");

            MV_LOOP(i)
            {
                if constexpr(has_named_components)
                    out += spec::component_names[i];
                else
                    out += std::to_string(i);

                out += ": " + std::to_string(data[i]) + (line_breaks ? '\n' : ' ');
            }
            return out;
        }
    };
}

/*
 *  type defines
*/

template<class T, unsigned N>
using math_vector = mv_impl::vector<T, N>;

MV_DEF_TYPES(2);
MV_DEF_TYPES(3);
MV_DEF_TYPES(4);

/*
 * static member resolves
*/

template<class T, unsigned N>
const math_vector<T, N> math_vector<T, N>::zero;

/*
 *  operator overloads
*/

MV_DEF_BIN_OP(+,  false);
MV_DEF_BIN_OP(-,  false);
MV_DEF_BIN_OP(*,  false);
MV_DEF_BIN_OP(/,  false);
MV_DEF_BIN_OP(%,  true);
MV_DEF_BIN_OP(&,  false);
MV_DEF_BIN_OP(|,  false);
MV_DEF_BIN_OP(^,  false);
MV_DEF_BIN_OP(>>, true);
MV_DEF_BIN_OP(<<, true);

MV_DEF_UN_OP(-);
MV_DEF_UN_OP(+);
MV_DEF_UN_OP(~);

// specialization for dividing vector by a scalar
template<class T, unsigned N, class S, class = std::enable_if_t<std::is_arithmetic_v<S>>>
MV_EXPR operator/(const math_vector<T, N>& v, S s)
{
    // ensure the division is only performed once
    return v * (1.0 / s);
}

/*
 *  vector util
*/

namespace mv_util
{
    // gets the point on the unit circle represented by the angle
    inline math_vector<double, 2> angle_coords(const double angle)
    {
        return math_vector<double, 2>
        {
            std::cos(angle),
            std::sin(angle)
        };
    }
}

/*
 *  std overloads and specializations
*/

namespace std
{
    template<class T, unsigned N>
    std::string to_string(const math_vector<T, N>& v)
    {
        return v.to_string(false);
    }

    // ostream specialization for use in ex. std::cout
    template<class T, unsigned N>
    std::ostream& operator<<(std::ostream& s, const math_vector<T, N>& v)
    {
        return s << v.to_string();
    }

    // hash specialization for use std::unordered_* containers
    template<class T, unsigned N>
    struct std::hash<math_vector<T, N>>
    {
        unsigned operator()(const math_vector<T, N>& v) const
        {
            std::string_view byte_data = { (char*)v.data, N * sizeof(T) };
            static std::hash<std::string_view> string_hash;

            return string_hash(byte_data);
        }
    };
}
