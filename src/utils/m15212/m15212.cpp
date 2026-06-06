#include "m15212/m15212.h"
QVector<double> m15212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
