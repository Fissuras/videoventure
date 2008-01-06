#pragma once

class Matrix2
{
public:
	Vector2 x;
	Vector2 y;
	Vector2 p;

public:

	Matrix2(void)
	{
	}

	Matrix2(const Vector2 &aX, const Vector2 &aY, const Vector2 &aP)
		: x(aX), y(aY), p(aP)
	{
	}

	Matrix2(const float aA, const Vector2 &aP)
		: x(cosf(aA), sinf(aA)), y(-x.y, x.x), p(aP)
	{
	}

	Matrix2(const b2Mat22 &aR, const b2Vec2 aP)
		: x(aR.col1), y(aR.col2), p(aP)
	{
	}

	~Matrix2(void)
	{
	}

	Matrix2 Inverse(void) const
	{
		float a = x.x, b = y.x, c = x.y, d = y.y;
		Matrix2 B;
		float det = a * d - b * c;
		det = 1.0f / det;
		B.x.x =  det * a;	B.y.x = -det * c;
		B.x.y = -det * b;	B.y.y =  det * d;
		B.p   = -p.x * x + -p.y * y;
		return B;
	}

	Vector2 Transform(const Vector2 &v) const
	{
		return x * v.x + y * v.y + p;
	}

	Vector2 Rotate(const Vector2 &v) const
	{
		return x * v.x + y * v.y;
	}

	float Angle(void) const
	{
		return -atan2f(y.x, y.y);
	}

	friend const Matrix2 operator*(const Matrix2 &a, const Matrix2 &b)
	{
		return Matrix2(
			a.x.x * b.x + a.x.y * b.y,
			a.y.x * b.x	+ a.y.y * b.y,
			a.p.x * b.x + a.p.y * b.y + b.p
			);
	}

	Matrix2 &operator*=(const Matrix2 &m)
	{
		*this = *this * m;
		return *this;
	}
};
