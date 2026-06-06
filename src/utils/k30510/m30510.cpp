#include "k30510/m30510.h"
QVector<double> m30510::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
