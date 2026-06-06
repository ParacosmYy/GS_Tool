#include "i9408/m9408.h"
QVector<double> m9408::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
