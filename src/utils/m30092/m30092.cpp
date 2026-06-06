#include "m30092/m30092.h"
QVector<double> m30092::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
