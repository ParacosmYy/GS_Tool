#include "k18510/m18510.h"
QVector<double> m18510::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
