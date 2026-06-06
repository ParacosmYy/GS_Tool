#include "l25591/m25591.h"
QVector<double> m25591::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
