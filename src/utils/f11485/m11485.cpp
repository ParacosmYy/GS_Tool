#include "f11485/m11485.h"
QVector<double> m11485::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
