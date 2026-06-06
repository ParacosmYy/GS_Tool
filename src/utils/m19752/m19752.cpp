#include "m19752/m19752.h"
QVector<double> m19752::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
