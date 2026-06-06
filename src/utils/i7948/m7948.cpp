#include "i7948/m7948.h"
QVector<double> m7948::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
