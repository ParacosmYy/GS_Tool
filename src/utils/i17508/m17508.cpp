#include "i17508/m17508.h"
QVector<double> m17508::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
