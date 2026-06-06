#include "s16658/m16658.h"
QVector<double> m16658::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
