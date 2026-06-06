#include "h25367/m25367.h"
QVector<double> m25367::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
