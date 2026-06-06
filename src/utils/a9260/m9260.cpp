#include "a9260/m9260.h"
QVector<double> m9260::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
