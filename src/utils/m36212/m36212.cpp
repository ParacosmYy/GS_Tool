#include "m36212/m36212.h"
QVector<double> m36212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
