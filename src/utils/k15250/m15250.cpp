#include "k15250/m15250.h"
QVector<double> m15250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
