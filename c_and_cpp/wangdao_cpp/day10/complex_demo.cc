#include <cmath>
#include <iostream>
#include <stdexcept>

class Complex {
private:
  double real_;
  double imag_;

public:
  // Constructors
  Complex() : real_(0), imag_(0) {}
  Complex(double real) : real_(real), imag_(0) {}
  Complex(double real, double imag) : real_(real), imag_(imag) {}

  // Getters
  double real() const { return real_; }
  double imag() const { return imag_; }
  double magnitude() const { return std::sqrt(real_ * real_ + imag_ * imag_); }

  // ============================================================
  // Unary Operators (member functions)
  // ============================================================

  // Negation: -c
  Complex operator-() const { return Complex(-real_, -imag_); }

  // Conjugate: ~c
  Complex operator~() const { return Complex(real_, -imag_); }

  explicit operator double() const {
    if (imag_ != 0) {
      throw std::runtime_error("Non-zero imaginary part");
    }
    return real_;
  }

  // ============================================================
  // Compound Assignment Operators (member functions)
  // ============================================================

  // +=
  Complex &operator+=(const Complex &other) {
    real_ += other.real_;
    imag_ += other.imag_;
    return *this;
  }

  // -=
  Complex &operator-=(const Complex &other) {
    real_ -= other.real_;
    imag_ -= other.imag_;
    return *this;
  }

  // *=
  Complex &operator*=(const Complex &other) {
    double newReal = real_ * other.real_ - imag_ * other.imag_;
    double newImag = real_ * other.imag_ + imag_ * other.real_;
    real_ = newReal;
    imag_ = newImag;
    return *this;
  }

  // /=
  Complex &operator/=(const Complex &other) {
    double denom = other.real_ * other.real_ + other.imag_ * other.imag_;
    double newReal = (real_ * other.real_ + imag_ * other.imag_) / denom;
    double newImag = (imag_ * other.real_ - real_ * other.imag_) / denom;
    real_ = newReal;
    imag_ = newImag;
    return *this;
  }

  // ============================================================
  // Increment/Decrement Operators (member functions)
  // ============================================================

  // Pre-increment: ++c
  Complex &operator++() {
    ++real_;
    return *this;
  }

  // Post-increment: c++
  Complex operator++(int) { // int is dummy parameter
    Complex temp = *this;
    ++real_;
    return temp;
  }

  // Pre-decrement: --c
  Complex &operator--() {
    --real_;
    return *this;
  }

  // Post-decrement: c--
  Complex operator--(int) {
    Complex temp = *this;
    --real_;
    return temp;
  }

  // ============================================================
  // Binary Operators (friend functions)
  // ============================================================

  // Addition: a + b
  friend Complex operator+(const Complex &a, const Complex &b) {
    return Complex(a.real_ + b.real_, a.imag_ + b.imag_);
  }

  // Subtraction: a - b
  friend Complex operator-(const Complex &a, const Complex &b) {
    return Complex(a.real_ - b.real_, a.imag_ - b.imag_);
  }

  // Multiplication: a * b
  // (a + bi)(c + di) = (ac - bd) + (ad + bc)i
  friend Complex operator*(const Complex &a, const Complex &b) {
    return Complex(a.real_ * b.real_ - a.imag_ * b.imag_,
                   a.real_ * b.imag_ + a.imag_ * b.real_);
  }

  // Division: a / b
  friend Complex operator/(const Complex &a, const Complex &b) {
    double denom = b.real_ * b.real_ + b.imag_ * b.imag_;
    return Complex((a.real_ * b.real_ + a.imag_ * b.imag_) / denom,
                   (a.imag_ * b.real_ - a.real_ * b.imag_) / denom);
  }

  // ============================================================
  // Comparison Operators (friend functions)
  // ============================================================

  // Equality: a == b
  friend bool operator==(const Complex &a, const Complex &b) {
    return a.real_ == b.real_ && a.imag_ == b.imag_;
  }

  // Inequality: a != b
  friend bool operator!=(const Complex &a, const Complex &b) {
    return !(a == b);
  }

  // ============================================================
  // Stream Operators (friend functions)
  // ============================================================

  // Output: cout << c
  friend std::ostream &operator<<(std::ostream &os, const Complex &c) {
    os << c.real_;
    if (c.imag_ >= 0) {
      os << " + " << c.imag_ << "i";
    } else {
      os << " - " << (-c.imag_) << "i";
    }
    return os;
  }

  // Input: cin >> c
  friend std::istream &operator>>(std::istream &is, Complex &c) {
    std::cout << "Enter real part: ";
    is >> c.real_;
    std::cout << "Enter imaginary part: ";
    is >> c.imag_;
    return is;
  }

  // ============================================================
  // Function Call Operator (member function)
  // ============================================================

  // Scale: c(factor)
  Complex operator()(double factor) const {
    return Complex(real_ * factor, imag_ * factor);
  }

  // ============================================================
  // Subscript Operator (member function)
  // ============================================================

  // Access: c[0] = real, c[1] = imag
  double &operator[](int index) {
    if (index == 0)
      return real_;
    if (index == 1)
      return imag_;
    throw std::out_of_range("Index must be 0 or 1");
  }

  const double &operator[](int index) const {
    if (index == 0)
      return real_;
    if (index == 1)
      return imag_;
    throw std::out_of_range("Index must be 0 or 1");
  }
};

// ============================================================
// Demo
// ============================================================

int main() {
  std::cout << "========================================\n";
  std::cout << "   Complex Number Operator Overloading\n";
  std::cout << "========================================\n\n";

  // Construction
  Complex a(3, 4);  // 3 + 4i
  Complex b(1, -2); // 1 - 2i
  Complex c(5);     // 5 + 0i (implicit conversion from double)
  Complex d;        // 0 + 0i

  std::cout << "--- Initial Values ---\n";
  std::cout << "a = " << a << "\n";
  std::cout << "b = " << b << "\n";
  std::cout << "c = " << c << "\n";
  std::cout << "d = " << d << "\n\n";

  // Arithmetic operators
  std::cout << "--- Arithmetic Operators ---\n";
  std::cout << "a + b = " << (a + b) << "\n";
  std::cout << "a - b = " << (a - b) << "\n";
  std::cout << "a * b = " << (a * b) << "\n";
  std::cout << "a / b = " << (a / b) << "\n\n";

  // Mixed with double (implicit conversion)
  std::cout << "--- Mixed Operations (implicit conversion) ---\n";
  std::cout << "a + 2 = " << (a + 2) << "\n";
  std::cout << "2 + a = " << (2 + a) << "\n";
  std::cout << "a * 3 = " << (a * 3) << "\n\n";

  // Unary operators
  std::cout << "--- Unary Operators ---\n";
  std::cout << "-a = " << (-a) << "\n";
  std::cout << "~a (conjugate) = " << (~a) << "\n\n";

  // Compound assignment
  std::cout << "--- Compound Assignment ---\n";
  Complex e = a;
  std::cout << "e = " << e << "\n";
  e += b;
  std::cout << "e += b: " << e << "\n";
  e -= b;
  std::cout << "e -= b: " << e << "\n";
  e *= Complex(2, 0);
  std::cout << "e *= 2: " << e << "\n\n";

  // Increment/Decrement
  std::cout << "--- Increment/Decrement ---\n";
  Complex f(3, 4);
  std::cout << "f = " << f << "\n";
  std::cout << "++f = " << (++f) << "\n";
  std::cout << "f++ = " << (f++) << "\n";
  std::cout << "f after f++ = " << f << "\n\n";

  // Comparison
  std::cout << "--- Comparison ---\n";
  Complex g(3, 4);
  Complex h(3, 4);
  Complex i(1, 2);
  std::cout << "g = " << g << ", h = " << h << ", i = " << i << "\n";
  std::cout << "g == h: " << (g == h ? "true" : "false") << "\n";
  std::cout << "g == i: " << (g == i ? "true" : "false") << "\n";
  std::cout << "g != i: " << (g != i ? "true" : "false") << "\n\n";

  // Subscript operator
  std::cout << "--- Subscript Operator ---\n";
  Complex j(5, 7);
  std::cout << "j = " << j << "\n";
  std::cout << "j[0] (real) = " << j[0] << "\n";
  std::cout << "j[1] (imag) = " << j[1] << "\n";
  j[0] = 10;
  j[1] = 20;
  std::cout << "After j[0]=10, j[1]=20: " << j << "\n\n";

  // Function call operator
  std::cout << "--- Function Call Operator ---\n";
  Complex k(2, 3);
  std::cout << "k = " << k << "\n";
  std::cout << "k(2) (scale by 2) = " << k(2) << "\n";
  std::cout << "k(0.5) (scale by 0.5) = " << k(0.5) << "\n\n";

  // Magnitude
  std::cout << "--- Magnitude ---\n";
  std::cout << "|a| = " << a.magnitude() << "\n";
  std::cout << "|3 + 4i| = " << Complex(3, 4).magnitude()
            << " (should be 5)\n\n";

  // Chaining
  std::cout << "--- Chaining Operations ---\n";
  Complex result = (a + b) * (a - b) / Complex(2, 0);
  std::cout << "(a + b) * (a - b) / 2 = " << result << "\n\n";

  std::cout << "========================================\n";
  std::cout << "   Demo Complete!\n";
  std::cout << "========================================\n";

  return 0;
}