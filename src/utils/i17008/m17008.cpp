#include "i17008/m17008.h"
QVector<double> m17008::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
