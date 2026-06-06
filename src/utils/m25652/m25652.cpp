#include "m25652/m25652.h"
QVector<double> m25652::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
