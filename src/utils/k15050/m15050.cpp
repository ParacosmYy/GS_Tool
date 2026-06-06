#include "k15050/m15050.h"
QVector<double> m15050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
