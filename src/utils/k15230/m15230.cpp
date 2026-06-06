#include "k15230/m15230.h"
QVector<double> m15230::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
