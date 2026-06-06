#include "k20250/m20250.h"
QVector<double> m20250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
