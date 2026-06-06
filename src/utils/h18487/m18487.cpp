#include "h18487/m18487.h"
QVector<double> m18487::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
