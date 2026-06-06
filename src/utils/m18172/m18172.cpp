#include "m18172/m18172.h"
QVector<double> m18172::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
