#include "k37510/m37510.h"
QVector<double> m37510::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
