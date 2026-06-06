#include "b8001/m8001.h"
QVector<double> m8001::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
