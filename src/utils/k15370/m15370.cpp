#include "k15370/m15370.h"
QVector<double> m15370::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
