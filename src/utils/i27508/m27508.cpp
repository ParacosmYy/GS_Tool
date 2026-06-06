#include "i27508/m27508.h"
QVector<double> m27508::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
