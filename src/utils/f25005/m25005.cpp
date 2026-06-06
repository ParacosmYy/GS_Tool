#include "f25005/m25005.h"
QVector<double> m25005::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
