#pragma once
#include "PCH.hpp"

class Vector2D
{
public:
    Vector2D();
    Vector2D(const double& x, const double& y);
    Vector2D(const Vector2D& other);

    void SetX(const double& x);
    double GetX() const;
    void SetY(const double& y);
    double GetY() const;

    void Rotate(const double& radians);

    Vector2D operator+(const Vector2D& other);
    void operator+=(const Vector2D& other);
    Vector2D operator+(const double& val);
    Vector2D operator-(const double& val);
    Vector2D operator-(const Vector2D& other);
    void operator-=(const Vector2D& other);
    Vector2D operator*(const Vector2D& other);
    Vector2D operator*(const double& scalar);
    bool operator==(const Vector2D& other);
    bool operator!=(const Vector2D& other);

    static Vector2D Normalize(Vector2D vec);
    static double Distance(Vector2D left, Vector2D right);
    static double Magnitude(Vector2D vec);

private:
    double m_x;
    double m_y;
};
