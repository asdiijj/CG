#pragma once
#ifndef POINT_H
#define POINT_H

class Point {
public:
	float x;
	float y;

	Point() {
		x = 0.0f;
		y = 0.0f;
	}
	Point(float x_, float y_) {
		x = x_;
		y = y_;
	}

	Point operator*(const float scalar) {
		return Point(this->x * scalar, this->y * scalar);
	}
	void operator*=(const float scalar) {
		this->x *= scalar;
		this->y *= scalar;
	}
	Point operator+(const Point& point) {
		return Point(this->x + point.x, this->y + point.y);
	}
	Point operator-(const Point& point) {
		return Point(this->x - point.x, this->y - point.y);
	}
	void operator+=(const Point& point) {
		this->x += point.x;
		this->y += point.y;
	}
};

#endif // !POINT_H

