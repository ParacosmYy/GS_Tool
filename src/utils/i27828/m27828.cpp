#include "i27828/m27828.h"
QVector<double> m27828::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
