#include "a26100/m26100.h"
QVector<double> m26100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
