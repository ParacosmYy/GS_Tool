#include "o16114/m16114.h"
QVector<double> m16114::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
