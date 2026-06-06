#include "e16904/m16904.h"
QVector<double> m16904::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
