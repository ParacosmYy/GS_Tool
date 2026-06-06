#include "b9041/m9041.h"
QVector<double> m9041::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
