#include "k15730/m15730.h"
QVector<double> m15730::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
