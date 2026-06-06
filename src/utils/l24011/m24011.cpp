#include "l24011/m24011.h"
QVector<double> m24011::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
