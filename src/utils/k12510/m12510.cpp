#include "k12510/m12510.h"
QVector<double> m12510::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
