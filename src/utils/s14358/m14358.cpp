#include "s14358/m14358.h"
QVector<double> m14358::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
