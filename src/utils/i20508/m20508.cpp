#include "i20508/m20508.h"
QVector<double> m20508::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
