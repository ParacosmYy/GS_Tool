#include "m28212/m28212.h"
QVector<double> m28212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
