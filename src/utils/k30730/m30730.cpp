#include "k30730/m30730.h"
QVector<double> m30730::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
