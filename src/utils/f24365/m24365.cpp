#include "f24365/m24365.h"
QVector<double> m24365::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
