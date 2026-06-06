#include "m37212/m37212.h"
QVector<double> m37212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
