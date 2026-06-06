#include "k25010/m25010.h"
QVector<double> m25010::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
