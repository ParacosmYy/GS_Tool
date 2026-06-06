#include "d17403/m17403.h"
QVector<double> m17403::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
