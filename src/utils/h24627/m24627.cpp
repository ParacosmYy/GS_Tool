#include "h24627/m24627.h"
QVector<double> m24627::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
