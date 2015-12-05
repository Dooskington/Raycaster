#pragma once

#include "PCH.hpp"

class Color {
    public:
        Color(const byte& red, const byte& green, const byte& blue, const byte& alpha);
        Color(const byte& red, const byte& green, const byte& blue);
        Color();

        void SetR(const byte& val);
        byte GetR();
        void SetG(const byte& val);
        byte GetG();
        void SetB(const byte& val);
        byte GetB();
        void SetA(const byte& val);
        byte GetA();

    private:
        byte m_red;
        byte m_green;
        byte m_blue;
        byte m_alpha;
};