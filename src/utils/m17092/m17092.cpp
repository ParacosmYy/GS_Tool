#include "m17092/m17092.h"
QVector<double> m17092::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
