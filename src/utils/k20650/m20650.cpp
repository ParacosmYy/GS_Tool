#include "k20650/m20650.h"
QVector<double> m20650::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
