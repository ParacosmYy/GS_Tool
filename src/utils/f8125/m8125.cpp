#include "f8125/m8125.h"
QVector<double> m8125::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
