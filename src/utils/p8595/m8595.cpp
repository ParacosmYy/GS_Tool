#include "p8595/m8595.h"
QVector<double> m8595::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
