#include "a19800/m19800.h"
QVector<double> m19800::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
