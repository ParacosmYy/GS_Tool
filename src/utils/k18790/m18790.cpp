#include "k18790/m18790.h"
QVector<double> m18790::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
