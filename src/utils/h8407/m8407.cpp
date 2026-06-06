#include "h8407/m8407.h"
QVector<double> m8407::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
