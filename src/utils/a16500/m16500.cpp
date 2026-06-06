#include "a16500/m16500.h"
QVector<double> m16500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
