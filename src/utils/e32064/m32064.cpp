#include "e32064/m32064.h"
QVector<double> m32064::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
