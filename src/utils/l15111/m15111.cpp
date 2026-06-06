#include "l15111/m15111.h"
QVector<double> m15111::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
