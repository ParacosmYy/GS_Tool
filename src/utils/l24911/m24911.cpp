#include "l24911/m24911.h"
QVector<double> m24911::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
