#include "i19808/m19808.h"
QVector<double> m19808::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
