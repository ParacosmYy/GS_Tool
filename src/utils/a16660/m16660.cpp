#include "a16660/m16660.h"
QVector<double> m16660::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
