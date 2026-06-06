#include "f20485/m20485.h"
QVector<double> m20485::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
