#include "p25535/m25535.h"
QVector<double> m25535::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
