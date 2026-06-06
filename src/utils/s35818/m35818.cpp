#include "s35818/m35818.h"
QVector<double> m35818::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
