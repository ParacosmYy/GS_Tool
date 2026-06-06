#include "i35508/m35508.h"
QVector<double> m35508::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
