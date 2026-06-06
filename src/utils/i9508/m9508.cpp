#include "i9508/m9508.h"
QVector<double> m9508::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
