#include "i25988/m25988.h"
QVector<double> m25988::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
