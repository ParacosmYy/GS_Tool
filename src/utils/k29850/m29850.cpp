#include "k29850/m29850.h"
QVector<double> m29850::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
