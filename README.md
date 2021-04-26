# math_vector

* Single-header
* N-dimensional
* Named components
* Lazily evaluated through use of [Expression Templates](https://en.wikipedia.org/wiki/Expression_templates)

NOTICE: It might be good to clarify that I made this library for fun and for use in various personal projects, so don't use it for anything super important as it might contain some bugs.

Also requires C++17 (or newer).

---

Vector class made to support dimensions up to UINT_MAX, with specialized access to individual components in common vector sizes (2 through 4). If, for example, you had a 2D vector, you could access its components either by index (`vector[0]`, `vector[1]`) or by name (`vector.x`, `vector.y`). Either methods will modify and access the same memory through use of references, so `vector[0] = 5` is equivalent to `vector.x = 5`. 

On top of that, I've implemented [Expression Templates](https://en.wikipedia.org/wiki/Expression_templates) as a way to optimize expressions involving vectors (let a, b, c be 2D vectors - the expresion `uint2d d = a + b * c` would compile to `for (uint i = 0; i < 2; i++) { d[i] = a[i] + b[i] * c[i]; }`, as opposed to allocating memory for and performing each calculation individually).

## Predefined types

| vec2d\<T> | vec3d\<T> | vec4d\<T> |
| :-: | :-: | :-: |
| int2d | int3d | int4d |
| uint2d | uint3d | uint4d |
| float2d | float3d | float4d |
| double2d | double3d | double4d |

And you can use the general syntax `math_vector<[type], [dimensions]>` for larger dimensions or with other base types.

## Operator overloads and math functions

There are operator overloads defined between either a ***vector/expression and a scalar*** (1) or a ***vector/expression and a vector/expression*** (2). Both are evaluated component-wise. (1) pairs up each component of the vector/expression with the scalar, and (2) pairs up each component in one vector/expression with its counterpart in the other.

| name | description |
| --- | --- |
| `length2()` | gets the length squared |
| `length()` | gets the length. equivalent to std::sqrt(length2()) |
| `normalize()` | gets vector divided by length. if length of vector is 0, returns vector |
| `scale_to(scalar)` | set vector length |
| `distance2(other_vector)` | gets the distance squared to another vector |
| `distance(other_vector)` | gets the distance to another vector. equivalent to std::sqrt(distance2(other_vector)) |
| `angle(other_vector)` | gets the angle to another vector |
| `dot(other_vector)` | gets the dot product with another vector |
| `sum()` | gets the sum of all components |
| `abs()` | gets the absolute value of all components |
| `cross(other_vector)` | for 3D vectors only, calculates the cross product with another vector |

## String serialization

Vectors are serialized to string through the `math_vector::to_string` function. Each component is printed with its respective component name or, if unavailable, its index. You can specify if the string should contain line-breaks between each component and if it should be prefixed with a name. 

## STL specializations and overloads

| name | description |
| --- | --- |
| `std::to_string` | equivalent to `vector.to_string`, without line-breaks |
| `std::ostream::operator<<` | equivalent to `stream << vector.to_string`, with line-breaks, for use with ex. `std::cout` |
| `std::hash::operator()` | hash specialization, for use in `std::unordered_*` containers |

## Improvements

* More thorough tests
* Return expressions from normalize, scale_to, and cross. Requires some restructuring since expr keeps a reference to its operands, as such any local variables (such as `len` in normalize) would go out of scope before the expression can be applied
