#include "i15868/m15868.h"
QVector<double> m15868::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
