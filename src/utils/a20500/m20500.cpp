#include "a20500/m20500.h"
QVector<double> m20500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
