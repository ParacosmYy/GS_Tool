#include "k29510/m29510.h"
QVector<double> m29510::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
