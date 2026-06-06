#include "s25658/m25658.h"
QVector<double> m25658::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
