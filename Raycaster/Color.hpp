#pragma once

#include "PCH.hpp"

class Color
{
    public:
        Color();
        Color(const byte& red, const byte& green, const byte& blue);
        Color(const byte& red, const byte& green, const byte& blue, const byte& alpha);

        void SetR(const byte& val);
        byte GetR() const;
        void SetG(const byte& val);
        byte GetG() const;
        void SetB(const byte& val);
        byte GetB() const;
        void SetA(const byte& val);
        byte GetA() const;

    private:
        byte m_red;
        byte m_green;
        byte m_blue;
        byte m_alpha;
};