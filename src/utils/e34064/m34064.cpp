#include "e34064/m34064.h"
QVector<double> m34064::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
