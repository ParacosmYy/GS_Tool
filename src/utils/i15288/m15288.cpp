#include "i15288/m15288.h"
QVector<double> m15288::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
