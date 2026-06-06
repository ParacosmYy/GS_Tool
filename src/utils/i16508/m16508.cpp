#include "i16508/m16508.h"
QVector<double> m16508::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
