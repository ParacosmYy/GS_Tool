#include "r9957/m9957.h"
QVector<double> m9957::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
