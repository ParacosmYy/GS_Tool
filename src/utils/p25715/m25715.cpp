#include "p25715/m25715.h"
QVector<double> m25715::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
