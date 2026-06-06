#include "f13485/m13485.h"
QVector<double> m13485::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
