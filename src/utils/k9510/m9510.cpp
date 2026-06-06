#include "k9510/m9510.h"
QVector<double> m9510::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
