#include "a27780/m27780.h"
QVector<double> m27780::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
