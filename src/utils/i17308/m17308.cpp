#include "i17308/m17308.h"
QVector<double> m17308::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
