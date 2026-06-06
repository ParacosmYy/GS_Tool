#include "p15115/m15115.h"
QVector<double> m15115::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
