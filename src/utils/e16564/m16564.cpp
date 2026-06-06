#include "e16564/m16564.h"
QVector<double> m16564::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
