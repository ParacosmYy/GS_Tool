#include "a18000/m18000.h"
QVector<double> m18000::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
