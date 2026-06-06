#include "i10208/m10208.h"
QVector<double> m10208::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
