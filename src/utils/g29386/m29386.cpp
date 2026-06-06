#include "g29386/m29386.h"
QVector<double> m29386::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
