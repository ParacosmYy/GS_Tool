#include "a24820/m24820.h"
QVector<double> m24820::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
