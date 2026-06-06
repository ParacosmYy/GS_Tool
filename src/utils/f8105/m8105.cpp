#include "f8105/m8105.h"
QVector<double> m8105::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
