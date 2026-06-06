#include "b35581/m35581.h"
QVector<double> m35581::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
