#include "s7998/m7998.h"
QVector<double> m7998::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
