#include "k11010/m11010.h"
QVector<double> m11010::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
