#include "PCH.hpp"
#include "Vector2D.hpp"

Vector2D::Vector2D()
{
    Vector2D(0, 0);
}

Vector2D::Vector2D(const double& x, const double& y) :
    m_x(x),
    m_y(y)
{
}

Vector2D::Vector2D(const Vector2D& other) :
    m_x(other.GetX()),
    m_y(other.GetY())
{
}

void Vector2D::SetX(const double& x)
{
    m_x = x;
}

double Vector2D::GetX() const
{
    return m_x;
}

void Vector2D::SetY(const double& y)
{
    m_y = y;
}

double Vector2D::GetY() const
{
    return m_y;
}

void Vector2D::Rotate(const double& degrees)
{
    double radians = degrees * (M_PI / 180.0);
    double newX = m_x * cos(radians) - m_y * sin(radians);
    double newY = m_x * sin(radians) + m_y * cos(radians);

    m_x = newX;
    m_y = newY;
}

// TODO 
// Refactor the following?

Vector2D Vector2D::operator+(const Vector2D& other)
{
    Vector2D vec;
    vec.m_x = m_x + other.m_x;
    vec.m_y = m_y + other.m_y;

    return vec;
}

void Vector2D::operator+=(const Vector2D& other)
{
    m_x += other.m_x;
    m_y += other.m_y;
}

Vector2D Vector2D::operator+(const double& val)
{
    Vector2D vec;
    vec.m_x = m_x + val;
    vec.m_y = m_y + val;

    return vec;
}

Vector2D Vector2D::operator-(const double& val)
{
    Vector2D vec;
    vec.m_x = m_x - val;
    vec.m_y = m_y - val;

    return vec;
}

Vector2D Vector2D::operator/(const double& val)
{
    Vector2D vec;
    vec.m_x = m_x / val;
    vec.m_y = m_y / val;

    return vec;
}

void Vector2D::operator/=(const double& val)
{
    m_x = m_x / val;
    m_y = m_y / val;
}

Vector2D Vector2D::operator-(const Vector2D& other)
{
    Vector2D vec;
    vec.m_x = m_x - other.m_x;
    vec.m_y = m_y - other.m_y;

    return vec;
}

void Vector2D::operator-=(const Vector2D& other)
{
    m_x = m_x - other.m_x;
    m_y = m_y - other.m_y;
}

Vector2D Vector2D::operator*(const Vector2D& other)
{
    Vector2D vec;
    vec.m_x = m_x * other.m_x;
    vec.m_y = m_y * other.m_y;

    return vec;
}

Vector2D Vector2D::operator*(const double& scalar)
{
    Vector2D vec;
    vec.m_x = m_x * scalar;
    vec.m_y = m_y * scalar;

    return vec;
}

bool Vector2D::operator==(const Vector2D& other)
{
    return (m_x == other.m_x && m_y == other.m_y);
}

bool Vector2D::operator!=(const Vector2D& other)
{
    return (m_x != other.m_x || m_y != other.m_y);
}

Vector2D Vector2D::Normalize(Vector2D vec)
{
    return Vector2D(vec.GetX() / Vector2D::Magnitude(vec), vec.GetY() / Vector2D::Magnitude(vec));
}

double Vector2D::Distance(Vector2D left, Vector2D right)
{
    return Vector2D::Magnitude(right - left);
}

double Vector2D::Magnitude(Vector2D vec)
{
    return sqrt((vec.GetX() * vec.GetX()) + (vec.GetY() * vec.GetY()));
}
