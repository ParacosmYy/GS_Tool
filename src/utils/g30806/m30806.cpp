#include "g30806/m30806.h"
QVector<double> m30806::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
