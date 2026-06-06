#include "g30306/m30306.h"
QVector<double> m30306::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
