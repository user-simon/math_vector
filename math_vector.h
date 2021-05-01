#pragma once
#include <string>
#include <tuple>
#include <type_traits>
#include <cmath>

// for explicit return types
#define MV_EXPR auto

// standard vector specialization definitions
#define MV_SPEC_DEFS(n, names) private:                                                         \
                                   using base = vector_spec<T, n - 1>;                          \
                               public:                                                          \
                                  constexpr static char component_names[] = names;              \
                                  vector_spec& operator= (const vector_spec&) { return *this; }

// defines a binary vector/expression and assignment operator
#define MV_DEF_OP(op) template<class L, class R,                                                \
                          class = std::enable_if_t<mv_impl::is_valid_op<L, R>>>                 \
                      MV_EXPR operator##op (const L& l, const R& r)                             \
                      {                                                                         \
                          return mv_impl::expr                                                  \
                          {                                                                     \
                            [](const auto& l, const auto& r) { return l op r; },                \
                            l, r                                                                \
                          };                                                                    \
                      }                                                                         \
                                                                                                \
                      template<class T, uint N, class R,                                        \
                          class = std::enable_if_t<mv_impl::is_valid_op<math_vector<T, N>, R>>> \
                      math_vector<T, N>& operator##op##=(math_vector<T, N>& l, const R& r)      \
                      {                                                                         \
                          return l = l op r;                                                    \
                      }

// utility for creating a unary expression from operation and operand
#define MV_UN_EXPR(op, val) mv_impl::expr([](const auto& v) { return op(v); }, val)

// defines a unary vector/expression operator
#define MV_DEF_UN_OP(op) template<class T, class = std::enable_if_t<mv_impl::is_vec_or_expr<T>>> \
                         MV_EXPR operator##op (const T& v)                                       \
                         {                                                                       \
                             return MV_UN_EXPR(op, v);                                           \
                         }

// defines a type and size alias for math_vector
#define MV_DEF_TYPE(t, n) using t##n##d = math_vec##n##d<t>

// defines type aliases for math_vector with specified size
#define MV_DEF_TYPES(n) template<class T> using math_vec##n##d = math_vector<T, n>; \
                        MV_DEF_TYPE(int,    n);                                     \
                        MV_DEF_TYPE(uint,   n);                                     \
                        MV_DEF_TYPE(float,  n);                                     \
                        MV_DEF_TYPE(double, n);                                     \

// utility for looping over all elements in vector with index
#define MV_LOOP(i) for (uint i = 0; i < N; i++)

/*
 *  arithmetic type aliases
*/

using uint  = unsigned int;
using ulong = unsigned long;
using uchar = unsigned char;

namespace mv_impl
{
    /*
     *  forward declares
    */

    template<class, class...>
    struct expr;

    template<class, uint>
    struct vector;

    /*
     *  is_vec_or_expr
     *      true if type is some instantiation of vector or expr
    */

    template<class>
    constexpr bool is_vec_or_expr = false;

    template<class OP, class... ARGS>
    constexpr bool is_vec_or_expr<expr<OP, ARGS...>> = true;

    template<class T, uint N>
    constexpr bool is_vec_or_expr<vector<T, N>> = true;

    /*
     *  is_valid_op
     *     true if the two types form a valid vector operation
    */

    template<class T, class U>
    constexpr bool is_valid_op = (is_vec_or_expr      <T> && is_vec_or_expr      <U>) ||
                                 (is_vec_or_expr      <T> && std::is_arithmetic_v<U>) ||
                                 (std::is_arithmetic_v<T> && is_vec_or_expr      <U>);
    
    /*
     *  has_named components
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
    struct expr
    {
    private:
        const OP& op;
        const std::tuple<const ARGS&...> args;

        // if T is array, get value at index i
        // otherwise, return value
        template<class T>
        static auto get_value(const T& v, const uint i)
        {
            if constexpr(is_vec_or_expr<T>)
                return v[i];
            else
                return v;
        }

    public:
        expr(const OP& op, const ARGS&... args) : op(op), args(args...) {}

        // evaluates value at index i
        auto inline operator[](const uint i) const
        {
            const auto& apply_op = [this, i](const ARGS&... a)
            {
                return op(get_value(a, i)...);
            };
            return std::apply(apply_op, args);
        }
    };

    /*
     *  vector_spec
     *      provides size-specific functionality of vector
    */

    template<class T, uint N>
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

#ifdef SFML
        template<typename U>
        operator sf::Vector2<U>() const
        {
            return sf::Vector2<U>(x, y);
        }
#endif
    };

    template<class T>
    struct vector_spec<T, 3> : vector_spec<T, 2>
    {
        MV_SPEC_DEFS(3, "xyz");

        using base::x;
        using base::y;
        T& z;

        constexpr vector_spec(T data[3]) : base(data), z(data[2]) {}

        // calculates the cross product two vectors
        template<typename U>
        constexpr vector<T, 3> cross(const vector<U, 3>& v) const
        {
            // TODO: return as expression if possible

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
    struct vector_spec<T, 4> : vector_spec<T, 3>
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

    template<typename T, uint N>
    struct vector : public vector_spec<T, N>
    {
        static_assert(std::is_arithmetic_v<T> && N > 1);
        
        using spec = vector_spec<T, N>;
        using type = T;
        constexpr static uint size = N;
        constexpr static bool has_named_components = has_named_components<vector>;
        const static vector zero;

        T data[N];

        /*
         *  ctors
        */
        
        // value ctor - sets all components to v
        constexpr vector(T v) : data{ v }, spec(data) {}

        // default ctor - sets all components to 0
        constexpr vector() : vector(0) {}
        
        // value ctor - specify a value for each component
        template<class...ARGS, class = std::enable_if_t<sizeof...(ARGS) == N && std::conjunction_v<std::is_arithmetic<ARGS>...>>>
        constexpr vector(ARGS... args) : data{ (T)args... }, spec(data) {}
        
        // copy ctor - copies values from other vector
        vector(const vector& v) : spec(data)
        {
            apply_values(v);
        }

        // application ctor - applies or copies values from another expression or vector, respectively
        template<class SRC, class = std::enable_if_t<is_vec_or_expr<SRC>>>
        vector(const SRC& src) : spec(data)
        {
            apply_values(src);
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
        T& operator[](const uint i)
        {
            return data[i];
        }

        // gets value at index i
        constexpr T operator[](const uint i) const
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

        // normalizes vector
        // returns *this if length == 0
        vector<double, 2> normalize() const
        {
            // TODO: return as expression if possible

            const double len = length();
            
            if (len == 0)
                return *this;
            return *this * (1.0 / len);
        }

        // sets length
        // equivalent to normalize() * s
        template<class U, class = std::enable_if_t<std::is_arithmetic_v<U>>>
        vector<double, 2> scale_to(U s) const
        {
            return normalize() * s;
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

        // calculates the angle between two vectors
        // if argument v is omitted, gets the absolute
        // angle of the vector
        template<class U>
        double angle(const vector<U, N>& v) const
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

        // calculates the absolute value of all components in vector
        // lazily evaluated
        MV_EXPR abs() const
        {
            return MV_UN_EXPR(std::abs, *this);
        }

        /*
         *  utility
        */

        // applies or copies values from another expression or vector, respectively
        template<class SRC, class = std::enable_if_t<is_vec_or_expr<SRC>>>
        void apply_values(const SRC& src)
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
        std::string to_string(bool line_breaks = true) const
        {
            std::string out = "";

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

        // serializes vector to string with name
        std::string to_string(const std::string& name, bool line_breaks = true) const
        {
            return name + (line_breaks ? "\n" : "  ") + to_string(line_breaks);
        }
    };
}

/*
 *  type defines
*/

template<class T, uint N>
using math_vector = mv_impl::vector<T, N>;

MV_DEF_TYPES(2);
MV_DEF_TYPES(3);
MV_DEF_TYPES(4);

/*
 * static member resolves
*/

template<class T, uint N>
const math_vector<T, N> math_vector<T, N>::zero;

/*
 *  operator overloads
*/

MV_DEF_OP(+);
MV_DEF_OP(-);
MV_DEF_OP(*);
MV_DEF_OP(/);

MV_DEF_UN_OP(+);
MV_DEF_UN_OP(-);

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
    template<class T, uint N>
    std::string to_string(const math_vector<T, N>& v)
    {
        return v.to_string(false);
    }

    // ostream specialization for use in ex. std::cout
    template<class T, uint N>
    std::ostream& operator<<(std::ostream& s, const math_vector<T, N>& v)
    {
        return s << v.to_string();
    }

    // hash specialization for use std::unordered_* containers
    template<typename T, uint N>
    struct std::hash<math_vector<T, N>>
    {
        uint operator()(const math_vector<T, N>& v) const
        {
            // since the algorithms used in hash specializations are implementation
            // dependant for STL types, the hash produced by this specialization
            // might vary between different compilers. in MSVC, it will use FNV-1a
            // to compute the hash

            std::string_view byte_data = { (char*)v.data, N * sizeof(T) };
            std::hash<std::string_view> string_hash;

            return string_hash(byte_data);
        }
    };
}
