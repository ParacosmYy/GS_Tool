#include "a11500/m11500.h"
QVector<double> m11500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
