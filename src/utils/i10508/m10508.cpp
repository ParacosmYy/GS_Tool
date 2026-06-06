#include "i10508/m10508.h"
QVector<double> m10508::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
