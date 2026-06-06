#include "k34510/m34510.h"
QVector<double> m34510::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
