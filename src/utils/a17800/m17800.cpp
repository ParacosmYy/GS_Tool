#include "a17800/m17800.h"
QVector<double> m17800::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
