#include "b35121/m35121.h"
QVector<double> m35121::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
