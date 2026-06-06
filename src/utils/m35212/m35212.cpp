#include "m35212/m35212.h"
QVector<double> m35212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
