#include "c9602/m9602.h"
QVector<double> m9602::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
