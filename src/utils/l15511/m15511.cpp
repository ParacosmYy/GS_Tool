#include "l15511/m15511.h"
QVector<double> m15511::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
