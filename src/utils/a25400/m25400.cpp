#include "a25400/m25400.h"
QVector<double> m25400::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
