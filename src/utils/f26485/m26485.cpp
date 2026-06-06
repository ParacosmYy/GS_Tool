#include "f26485/m26485.h"
QVector<double> m26485::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
