#include "i9368/m9368.h"
QVector<double> m9368::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
