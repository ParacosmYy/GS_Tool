#include "m30372/m30372.h"
QVector<double> m30372::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
