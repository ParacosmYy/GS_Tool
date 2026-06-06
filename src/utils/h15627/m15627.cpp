#include "h15627/m15627.h"
QVector<double> m15627::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
