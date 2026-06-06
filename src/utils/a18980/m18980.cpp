#include "a18980/m18980.h"
QVector<double> m18980::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
