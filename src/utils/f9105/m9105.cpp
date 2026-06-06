#include "f9105/m9105.h"
QVector<double> m9105::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
