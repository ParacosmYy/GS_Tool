#include "d7943/m7943.h"
QVector<double> m7943::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
