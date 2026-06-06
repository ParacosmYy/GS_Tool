#include "d16123/m16123.h"
QVector<double> m16123::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
