#include "c18202/m18202.h"
QVector<double> m18202::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
