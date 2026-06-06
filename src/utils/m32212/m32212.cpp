#include "m32212/m32212.h"
QVector<double> m32212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
