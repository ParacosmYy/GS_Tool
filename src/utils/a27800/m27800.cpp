#include "a27800/m27800.h"
QVector<double> m27800::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
