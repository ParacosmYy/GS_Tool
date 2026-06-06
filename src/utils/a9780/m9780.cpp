#include "a9780/m9780.h"
QVector<double> m9780::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
