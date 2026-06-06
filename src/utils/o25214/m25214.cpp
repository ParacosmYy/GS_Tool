#include "o25214/m25214.h"
QVector<double> m25214::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
