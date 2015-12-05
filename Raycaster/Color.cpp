#include "Color.hpp"

Color::Color(const byte& red, const byte& green, const byte& blue, const byte& alpha) :
    m_red(red),
    m_green(green),
    m_blue(blue),
    m_alpha(alpha)
{
}

Color::Color(const byte& red, const byte& green, const byte& blue) :
    m_red(red),
    m_green(green),
    m_blue(blue)
{
    Color(red, green, blue, 255);
}

Color::Color()
{
    Color(0, 0, 0);
}

void Color::SetR(const byte& val)
{
    m_red = val;
}

byte Color::GetR()
{
    return m_red;
}

void Color::SetG(const byte& val)
{
    m_green = val;
}

byte Color::GetG()
{
    return m_green;
}

void Color::SetB(const byte& val)
{
    m_blue = val;
}

byte Color::GetB()
{
    return m_blue;
}

void Color::SetA(const byte& val)
{
    m_alpha = val;
}

byte Color::GetA()
{
    return m_alpha;
}