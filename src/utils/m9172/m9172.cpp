#include "m9172/m9172.h"
QVector<double> m9172::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
