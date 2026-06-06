#include "l8451/m8451.h"
QVector<double> m8451::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
