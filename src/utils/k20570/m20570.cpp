#include "k20570/m20570.h"
QVector<double> m20570::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
