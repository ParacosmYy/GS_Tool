#include "k31510/m31510.h"
QVector<double> m31510::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
