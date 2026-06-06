#include "a18120/m18120.h"
QVector<double> m18120::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
