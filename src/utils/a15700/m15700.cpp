#include "a15700/m15700.h"
QVector<double> m15700::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
