#include "d16783/m16783.h"
QVector<double> m16783::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
