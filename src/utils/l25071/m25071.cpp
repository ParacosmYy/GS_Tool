#include "l25071/m25071.h"
QVector<double> m25071::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
