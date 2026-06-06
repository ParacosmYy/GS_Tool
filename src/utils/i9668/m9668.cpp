#include "i9668/m9668.h"
QVector<double> m9668::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
