#include "m36912/m36912.h"
QVector<double> m36912::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
