#include "k15150/m15150.h"
QVector<double> m15150::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
