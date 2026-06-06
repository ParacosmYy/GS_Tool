#include "b24101/m24101.h"
QVector<double> m24101::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
