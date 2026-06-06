#include "i24488/m24488.h"
QVector<double> m24488::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
