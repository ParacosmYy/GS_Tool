#include "i16408/m16408.h"
QVector<double> m16408::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
