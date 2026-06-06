#include "b8881/m8881.h"
QVector<double> m8881::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
