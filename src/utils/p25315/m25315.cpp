#include "p25315/m25315.h"
QVector<double> m25315::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
