#include "m27212/m27212.h"
QVector<double> m27212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
