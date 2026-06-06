#include "o16414/m16414.h"
QVector<double> m16414::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
