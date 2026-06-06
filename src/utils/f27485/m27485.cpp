#include "f27485/m27485.h"
QVector<double> m27485::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
