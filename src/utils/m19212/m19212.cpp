#include "m19212/m19212.h"
QVector<double> m19212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
