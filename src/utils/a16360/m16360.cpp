#include "a16360/m16360.h"
QVector<double> m16360::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
