#include "b9501/m9501.h"
QVector<double> m9501::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
