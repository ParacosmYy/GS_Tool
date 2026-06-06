#include "o15454/m15454.h"
QVector<double> m15454::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
