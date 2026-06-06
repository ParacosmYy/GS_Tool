#include "k9070/m9070.h"
QVector<double> m9070::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
