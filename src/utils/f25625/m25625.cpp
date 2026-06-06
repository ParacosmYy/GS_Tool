#include "f25625/m25625.h"
QVector<double> m25625::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
