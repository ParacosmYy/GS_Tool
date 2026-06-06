#include "m18212/m18212.h"
QVector<double> m18212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
