#include "k20330/m20330.h"
QVector<double> m20330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
