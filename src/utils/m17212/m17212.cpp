#include "m17212/m17212.h"
QVector<double> m17212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
