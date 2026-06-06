#include "f16485/m16485.h"
QVector<double> m16485::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
