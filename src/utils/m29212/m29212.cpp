#include "m29212/m29212.h"
QVector<double> m29212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
