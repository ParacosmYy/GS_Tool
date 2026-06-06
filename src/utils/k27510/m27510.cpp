#include "k27510/m27510.h"
QVector<double> m27510::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
