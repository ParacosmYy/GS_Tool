#include "s9358/m9358.h"
QVector<double> m9358::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
