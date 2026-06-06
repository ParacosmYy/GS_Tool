#include "m20212/m20212.h"
QVector<double> m20212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
