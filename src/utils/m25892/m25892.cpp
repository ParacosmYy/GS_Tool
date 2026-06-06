#include "m25892/m25892.h"
QVector<double> m25892::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
