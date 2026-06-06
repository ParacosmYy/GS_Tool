#include "i17808/m17808.h"
QVector<double> m17808::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
