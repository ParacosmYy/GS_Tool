#include "a13500/m13500.h"
QVector<double> m13500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
