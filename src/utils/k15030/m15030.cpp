#include "k15030/m15030.h"
QVector<double> m15030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
