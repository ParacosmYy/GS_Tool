#include "k20810/m20810.h"
QVector<double> m20810::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
