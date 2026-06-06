#include "l16451/m16451.h"
QVector<double> m16451::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
