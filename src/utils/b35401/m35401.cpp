#include "b35401/m35401.h"
QVector<double> m35401::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
