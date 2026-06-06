#include "m24092/m24092.h"
QVector<double> m24092::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
