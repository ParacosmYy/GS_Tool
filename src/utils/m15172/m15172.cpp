#include "m15172/m15172.h"
QVector<double> m15172::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
