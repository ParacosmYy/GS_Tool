#include "p24715/m24715.h"
QVector<double> m24715::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
