#include "k25070/m25070.h"
QVector<double> m25070::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
