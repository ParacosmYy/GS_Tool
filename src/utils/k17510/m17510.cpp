#include "k17510/m17510.h"
QVector<double> m17510::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
