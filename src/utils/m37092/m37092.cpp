#include "m37092/m37092.h"
QVector<double> m37092::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
