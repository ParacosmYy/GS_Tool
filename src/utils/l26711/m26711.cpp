#include "l26711/m26711.h"
QVector<double> m26711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
