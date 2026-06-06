#include "s35358/m35358.h"
QVector<double> m35358::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
