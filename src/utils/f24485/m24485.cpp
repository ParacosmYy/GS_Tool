#include "f24485/m24485.h"
QVector<double> m24485::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
