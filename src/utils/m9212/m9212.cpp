#include "m9212/m9212.h"
QVector<double> m9212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
