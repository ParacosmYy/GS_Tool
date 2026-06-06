#include "p25275/m25275.h"
QVector<double> m25275::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
