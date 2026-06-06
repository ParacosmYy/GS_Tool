#include "l24511/m24511.h"
QVector<double> m24511::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
