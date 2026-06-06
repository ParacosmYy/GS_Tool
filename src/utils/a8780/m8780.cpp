#include "a8780/m8780.h"
QVector<double> m8780::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
