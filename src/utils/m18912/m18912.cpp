#include "m18912/m18912.h"
QVector<double> m18912::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
