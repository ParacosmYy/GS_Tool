#include "f10485/m10485.h"
QVector<double> m10485::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
