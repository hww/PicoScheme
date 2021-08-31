/*********************************************************************************/ /**
 * @file number.hpp
 *
 * @version   0.1
 * @date      2018-
 * @author    Paul Pudewills
 * @copyright MIT License
 *************************************************************************************/
#ifndef NUMBER_HPP
#define NUMBER_HPP

#include <complex>
#include <iostream>
#include <variant>

#include "utils.hpp"

namespace pscm {

using Int = int64_t;
using Float = double;
using Complex = std::complex<double>;

template <typename T>
constexpr T pi = 3.141592653589793238462643383279502884197169399375105820974944592307;

template <typename T>
constexpr T e = 2.718281828459045235360287471352662497757247093699959574966967627724;

template <typename T> // Gravitational constant [m^3/(kg s^2)]
constexpr T G = 6.67408e-11;

template <typename T> // Speed of light [s]
constexpr T c = 299792458;

template <typename T> // Planck's constant [Js]
constexpr T h = 6.62607015081e-34;

template <typename T> // Elementary electric charge [C]
constexpr T q_e = 1.602176620898e-19;

template <typename T> //  Avogadro number [1/mol]
constexpr T N_A = 6.02214076e23;

template <typename T> // Gas constant [J/(mol K)]
constexpr T R = 8.314459848;

template <typename T> // Vacuum permeability [C^2 / (N m^2)]
constexpr T mu_0 = pi<T> * 4 * 1e-7;

template <typename T> // Vacuum permittivity [(N m^2)/C^2]
constexpr T epsilon_0 = 1 / (mu_0<T> * c<T> * c<T>);

template <typename T> // Stefan-Boltzmann constant [W/(m^2 K^4)]
constexpr T sigma = 2 * pi<T>* R<T>* R<T>* R<T>* R<T> / (15 * h<T> * h<T> * h<T> * c<T> * c<T> * N_A<T> * N_A<T> * N_A<T> * N_A<T>);

template <typename T>
struct Type;

/**
 * @brief Number struct as union of integer, floating point and complex numbers.
 *
 * A floating point number it converted into an integer if it is
 * exact representable as an integer. If the imaginary part of a complex
 * number is zero, only a number representing the real part is
 * constructed.
 */
struct Number : std::variant<Int, Float, Complex> {
    using base_type = std::variant<Int, Float, Complex>;
    using base_type::operator=;

    constexpr Number()
        : base_type{ Int{ 0 } }
    {
    }

    Number(const Number&) = default;
    Number& operator=(const Number&) = default;
    Number& operator=(Number&&) = default;

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    constexpr Number(T x)
        : base_type{ static_cast<Int>(x) }
    {
    }

    constexpr Number(Float x)
        : base_type{ x }
    {
    }
    /**
     * Converting constructor for complex type arguments.
     */
    constexpr Number(const Complex& z)
        : Number{ z.real(), z.imag() }
    {
    }

    template <typename RE, typename IM>
    constexpr Number(RE x, IM y)
    {
        if (y > IM{ 0 } || y < IM{ 0 })
            *this = base_type{ Complex{ static_cast<Float>(x), static_cast<Float>(y) } };
        else
            *this = Number{ x };
    }

    /**
     * Conversion operator to convert a Number type to the requested arithmetic or complex type.
     */
    template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T> || std::is_same_v<T, Complex>>>
    explicit constexpr operator T() const noexcept
    {
        auto fun = [](auto& num) -> T {
            using TT = std::decay_t<decltype(num)>;

            if constexpr (std::is_same_v<TT, Int>) {
                if constexpr (std::is_integral_v<T>)
                    return static_cast<T>(num);

                else if constexpr (std::is_floating_point_v<T>)
                    return static_cast<T>(num);

                else // T is Complex:
                    return static_cast<T>(static_cast<typename T::value_type>(num));

            } else if constexpr (std::is_same_v<TT, Float>) {
                if constexpr (std::is_arithmetic_v<T>)
                    return static_cast<T>(num);

                else // T is Complex
                    return static_cast<T>(static_cast<typename T::value_type>(num));

            } else if constexpr (std::is_same_v<TT, Complex>) {
                if constexpr (std::is_arithmetic_v<T>)
                    return static_cast<T>(std::abs(num));

                else
                    return static_cast<T>(num);
            } else
                static_assert(always_false<TT>::value, "invalid variant");
        };

        return std::visit(std::move(fun), static_cast<const base_type&>(*this));
    }

    struct hash {
        using argument_type = pscm::Number;
        using result_type = std::size_t;

        result_type operator()(const Number& num) const
        {
            static overloads hash{
                [](const Complex& z) -> result_type {
                    auto a = std::hash<Complex::value_type>{}(z.real());
                    auto b = std::hash<Complex::value_type>{}(z.imag());
                    auto c = std::hash<Complex::value_type>{}(std::abs(z)) + 0x765432F;
                    c ^= a + 0x9e3779b9 + (c << 6) + (c >> 2);
                    c ^= b + 0x9e3779b9 + (c << 6) + (c >> 2);
                    return c;
                },
                [](auto arg) -> result_type { return std::hash<decltype(arg)>{}(arg); },
            };
            return visit(hash, static_cast<const Number::base_type&>(num));
        }
    };
};

template <typename T>
Number num(const T& x) { return { x }; }

template <typename RE, typename IM>
Number num(const RE& x, const IM& y) { return { x, y }; }

inline bool is_int(const Number& num) { return is_type<Int>(num); }
inline bool is_float(const Number& num) { return is_type<Float>(num); }
inline bool is_complex(const Number& num) { return is_type<Complex>(num); }

bool is_integer(const Number& num);
bool is_odd(const Number& num);

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const Complex& z)
{
    if (auto im = z.imag(); im > 0 || im < 0) {
        if (im < 0) {
            os << z.real();
            if (im > -1 || im < -1)
                return os << '-' << im << 'i';
            else
                return os << "-i";

        } else if (im > 1 || im < 1)
            return os << z.real() << '+' << im << 'i';
        else
            return os << z.real() << "+i";
    } else
        return os << z.real();
}

/**
 * @brief Out stream operator for a ::Number argument value.
 */
template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const Number& num)
{
    return std::visit([&os](auto x) -> decltype(os) {
        if constexpr (std::is_same_v<Int, decltype(x)>)
            return os << x;
        else
            return os << std::scientific << x;
    },
        static_cast<Number::base_type>(num));
}

bool operator!=(const Number& lhs, const Number& rhs);
bool operator==(const Number& lhs, const Number& rhs);
bool operator>(const Number& lhs, const Number& rhs);
bool operator<(const Number& lhs, const Number& rhs);
bool operator>=(const Number& lhs, const Number& rhs);
bool operator<=(const Number& lhs, const Number& rhs);

Number min(const Number& lhs, const Number& rhs);
Number max(const Number& lhs, const Number& rhs);

inline bool is_zero(const Number& x) { return !(x != Number{ 0 }); }
inline bool is_negative(const Number& x) { return x < Number{ 0 }; }
inline bool is_positive(const Number& x) { return x > Number{ 0 }; }

Number inv(const Number& x);

Number operator-(const Number& x);
Number operator+(const Number& lhs, const Number& rhs);
Number operator-(const Number& lhs, const Number& rhs);
Number operator*(const Number& lhs, const Number& rhs);
Number operator/(const Number& lhs, const Number& rhs);
Number operator%(const Number& lhs, const Number& rhs);

Number& operator+=(Number& lhs, const Number& rhs);
Number& operator-=(Number& lhs, const Number& rhs);
Number& operator*=(Number& lhs, const Number& rhs);
Number& operator/=(Number& lhs, const Number& rhs);

Number round(const Number& x);
Number floor(const Number& x);
Number ceil(const Number& x);
Number trunc(const Number& x);
Number remainder(const Number& lhs, const Number& rhs);
Number quotient(const Number& lhs, const Number& rhs);

Number sin(const Number& x);
Number cos(const Number& x);
Number tan(const Number& x);
Number asin(const Number& x);
Number acos(const Number& x);
Number atan(const Number& x);
Number sinh(const Number& x);
Number cosh(const Number& x);
Number tanh(const Number& x);
Number asinh(const Number& x);
Number acosh(const Number& x);
Number atanh(const Number& x);

Number exp(const Number& x);
Number log(const Number& x);
Number log10(const Number& x);
Number sqrt(const Number& x);
Number cbrt(const Number& x);
Number pow(const Number& x, const Number& y);

Number abs(const Number& x);
Number real(const Number& z);
Number imag(const Number& z);
Number arg(const Number& z);
Number conj(const Number& z);
Number rect(const Number& x, const Number& y);
Number polar(const Number& r, const Number& theta);
Number hypot(const Number& x, const Number& y);
Number hypot(const Number& x, const Number& y, const Number& z);

} // namspace pscm

#endif // NUMBER_HPP
