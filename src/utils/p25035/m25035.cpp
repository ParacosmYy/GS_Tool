#include "p25035/m25035.h"
QVector<double> m25035::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
