#include "m25172/m25172.h"
QVector<double> m25172::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
