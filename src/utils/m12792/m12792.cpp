#include "m12792/m12792.h"
QVector<double> m12792::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
