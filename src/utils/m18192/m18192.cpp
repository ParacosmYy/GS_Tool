#include "m18192/m18192.h"
QVector<double> m18192::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
