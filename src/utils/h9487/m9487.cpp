#include "h9487/m9487.h"
QVector<double> m9487::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
