#include "q31556/m31556.h"
QVector<double> m31556::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
