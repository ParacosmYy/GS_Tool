#include "h24007/m24007.h"
QVector<double> m24007::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
