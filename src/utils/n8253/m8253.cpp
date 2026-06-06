#include "n8253/m8253.h"
QVector<double> m8253::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
