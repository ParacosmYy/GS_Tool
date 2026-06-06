#include "i24328/m24328.h"
QVector<double> m24328::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
