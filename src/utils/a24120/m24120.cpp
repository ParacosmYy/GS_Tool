#include "a24120/m24120.h"
QVector<double> m24120::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
