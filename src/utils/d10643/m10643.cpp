#include "d10643/m10643.h"
QVector<double> m10643::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
