//
// Created by armado on 9/20/22.
//

#ifndef GUNMO_MATH_HPP
#define GUNMO_MATH_HPP

namespace Math {
	template<typename T>
	class Vec {

		void _add(Vec<T> &val) {
			x += val.x;
			y += val.y;
		}
		void _sub(Vec<T> &val) {
			x -= val.x;
			y -= val.y;
		}
		void _mult(T val) {
			x *= val;
			y *= val;
		}
		void _div(T val) {
			x /= val;
			y /= val;
		}

	public:
		T x = 0;
		T y = 0;

		void operator +=(Vec<T> &rhs) {
			_add(rhs);
		}

		void operator -=(Vec<T> &rhs) {
			_sub(rhs);
		}

		void operator *=(Vec<T> &rhs) {
			_mult(rhs);
		}

		void operator /=(Vec<T> &rhs) {
			_div(rhs);
		}

		Vec<T> operator + (Vec<T> &val) {
			auto cpy = (*this);
			cpy += val;
			return cpy;
		}

		Vec<T> operator - (Vec<T> &val) {
			auto cpy = (*this);
			cpy -= val;
			return cpy;
		}

		Vec<T> operator * (Vec<T> &val) {
			auto cpy = (*this);
			cpy *= val;
			return cpy;
		}

		Vec<T> operator / (Vec<T> &val) {
			auto cpy = (*this);
			cpy /= val;
			return cpy;
		}

	};
}

#endif //GUNMO_MATH_HPP
