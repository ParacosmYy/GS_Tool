#include "k20770/m20770.h"
QVector<double> m20770::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
