#include "a17500/m17500.h"
QVector<double> m17500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
