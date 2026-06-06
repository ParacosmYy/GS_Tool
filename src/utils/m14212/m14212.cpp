#include "m14212/m14212.h"
QVector<double> m14212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
