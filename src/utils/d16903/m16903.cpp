#include "d16903/m16903.h"
QVector<double> m16903::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
