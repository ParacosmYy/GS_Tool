#include "s15358/m15358.h"
QVector<double> m15358::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
