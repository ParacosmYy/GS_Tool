#include "m36092/m36092.h"
QVector<double> m36092::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
