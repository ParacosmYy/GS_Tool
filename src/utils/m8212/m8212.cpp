#include "m8212/m8212.h"
QVector<double> m8212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
