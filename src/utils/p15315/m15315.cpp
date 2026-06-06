#include "p15315/m15315.h"
QVector<double> m15315::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
