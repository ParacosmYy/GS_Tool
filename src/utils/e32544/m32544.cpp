#include "e32544/m32544.h"
QVector<double> m32544::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
