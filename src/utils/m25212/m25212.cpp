#include "m25212/m25212.h"
QVector<double> m25212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
