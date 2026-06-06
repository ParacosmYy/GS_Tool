#include "a23500/m23500.h"
QVector<double> m23500::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
