#include "a25780/m25780.h"
QVector<double> m25780::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
