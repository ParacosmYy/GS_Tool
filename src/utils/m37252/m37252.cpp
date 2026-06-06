#include "m37252/m37252.h"
QVector<double> m37252::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
