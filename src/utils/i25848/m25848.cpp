#include "i25848/m25848.h"
QVector<double> m25848::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
