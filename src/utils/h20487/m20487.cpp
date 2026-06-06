#include "h20487/m20487.h"
QVector<double> m20487::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
