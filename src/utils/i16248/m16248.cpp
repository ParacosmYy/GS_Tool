#include "i16248/m16248.h"
QVector<double> m16248::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
