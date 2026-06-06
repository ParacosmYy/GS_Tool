#include "l29111/m29111.h"
QVector<double> m29111::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
