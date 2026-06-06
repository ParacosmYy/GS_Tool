#include "m9792/m9792.h"
QVector<double> m9792::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
