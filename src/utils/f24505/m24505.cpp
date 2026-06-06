#include "f24505/m24505.h"
QVector<double> m24505::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
