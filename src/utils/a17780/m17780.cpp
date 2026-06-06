#include "a17780/m17780.h"
QVector<double> m17780::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
