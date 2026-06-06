#include "m20412/m20412.h"
QVector<double> m20412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
